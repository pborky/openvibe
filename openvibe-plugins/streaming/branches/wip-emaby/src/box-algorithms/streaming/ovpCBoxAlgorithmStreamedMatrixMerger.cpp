#include "ovpCBoxAlgorithmStreamedMatrixMerger.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cfloat>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Streaming;
using namespace std;
using namespace OpenViBEToolkit;


boolean CBoxAlgorithmStreamedMatrixMerger::initialize(void)
{
	
	
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	m_ui64NbEntries = getStaticBoxContext().getInputCount();
	
	// Streamed Matrix stream decoder
	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{	
		m_vStrMatDecoders.push_back( new TStreamedMatrixDecoder <CBoxAlgorithmStreamedMatrixMerger> (*this) );
	}
	m_pNbSamplesPerInput.resize(l_rStaticBoxContext.getInputCount());			
	// Streamed Matrix stream encoder
	m_oStreamedMatrixEncoder.initialize(*this);
	m_pOutputMatrix=m_oStreamedMatrixEncoder.getInputMatrix();
	m_ui64NbChannelsOutput= 0;
	l_ui32SampleCountPerSentBlock= 0;
	m_ui64NbEntriesDecoded = 0;

	return true;
}

boolean CBoxAlgorithmStreamedMatrixMerger::uninitialize(void)
{
	for(uint32 i = 0; i< m_vStrMatDecoders.size(); i++)
	{
		m_vStrMatDecoders[i]->uninitialize();
	}
		
	m_oStreamedMatrixEncoder.uninitialize();

	return true;
}

boolean CBoxAlgorithmStreamedMatrixMerger::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

boolean CBoxAlgorithmStreamedMatrixMerger::process(void)
{
	IBox& l_rStaticBoxContext=getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=getDynamicBoxContext();
	

	for(uint32 i=0; i<l_rStaticBoxContext.getInputCount(); i++)
	{
		for(uint32 j=0; j<l_rDynamicBoxContext.getInputChunkCount(i); j++)
		{
			m_vStrMatDecoders[i]->decode(i,j);
			if(m_vStrMatDecoders[i]->isHeaderReceived())
			{
				// initialize the past data array
				uint32 l_ui32DimensionCount;
				IMatrix* l_pMatrix = ( (TStreamedMatrixDecoder <CBoxAlgorithmStreamedMatrixMerger> *) m_vStrMatDecoders[i] )->getOutputMatrix();
				l_ui32DimensionCount = l_pMatrix->getDimensionCount();
				

				if(l_pMatrix->getDimensionCount() != 2)
				{
					this->getLogManager() << LogLevel_Error << "Streamed matrix dimension count is not 2\n";
					return false;
				}
				
				m_pNbSamplesPerInput[i] = l_pMatrix->getDimensionSize(0)*l_pMatrix->getDimensionSize(1);
				m_ui64NbChannelsOutput = m_ui64NbChannelsOutput + l_pMatrix->getDimensionSize(0);
					

				m_pOutputMatrix->setDimensionCount(2);
				m_pOutputMatrix->setDimensionSize(0, m_ui64NbChannelsOutput);
				m_pOutputMatrix->setDimensionSize(1, l_pMatrix->getDimensionSize(1));
				Tools::Matrix::clearContent(*m_pOutputMatrix);
				
				m_pDataEntries.resize(m_ui64NbChannelsOutput*l_pMatrix->getDimensionSize(1));
					
				
				
						

				// Encode the output header
				m_oStreamedMatrixEncoder.encodeHeader(0);
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(i, j), l_rDynamicBoxContext.getInputChunkEndTime(i, j));
			}

			
			
			
			
			if(m_vStrMatDecoders[i]->isBufferReceived())
			{
				IMatrix* l_pMatrix = ( (TStreamedMatrixDecoder <CBoxAlgorithmStreamedMatrixMerger> *) m_vStrMatDecoders[i] )->getOutputMatrix();

				uint32 l_ui32ChannelCount      = l_pMatrix->getDimensionSize(0);
				uint32 l_ui32SamplesPerChannel = l_pMatrix->getDimensionSize(1);
				

				float64* l_pBuffer = l_pMatrix->getBuffer();

				uint32 Cumul_samples = 0;
				
				for (uint32 l_chan = 0; l_chan < i; l_chan++)
				{
					Cumul_samples = Cumul_samples + m_pNbSamplesPerInput[l_chan];
				}
				

				for (uint32 channel = 0; channel < l_ui32ChannelCount; channel++)
				{
					for (uint32 sample = 0; sample < l_ui32SamplesPerChannel; sample++)
					{
						m_pDataEntries[Cumul_samples + sample + channel * l_ui32SamplesPerChannel]= l_pBuffer[sample + channel * l_ui32SamplesPerChannel];
					}
				}
				
				
				

				m_ui64NbEntriesDecoded=m_ui64NbEntriesDecoded+1;
				
				if (m_ui64NbEntriesDecoded==m_ui64NbEntries)
				{					
					uint32 k_samp = 0;
					for (uint32 channel = 0; channel < m_pOutputMatrix->getDimensionSize(0); channel++)
					{
						for (uint32 sample = 0; sample < m_pOutputMatrix->getDimensionSize(1); sample++)
						{
							m_pOutputMatrix->getBuffer()[k_samp]=m_pDataEntries[k_samp];
							k_samp = k_samp + 1;
						}
					}

					
					m_oStreamedMatrixEncoder.encodeBuffer(0);
					l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(i, j), l_rDynamicBoxContext.getInputChunkEndTime(i, j));
					m_ui64NbEntriesDecoded=0;
					m_ui64NbChannelsOutput=0;
					
					m_pDataEntries.clear();
					
				}				
			}
			
			
			if(m_vStrMatDecoders[i]->isEndReceived())
			{
				// End of stream received. This happens only once when pressing "stop". Just pass it to the next boxes so they receive the message :
				m_oStreamedMatrixEncoder.encodeEnd(0);
				l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(i, j), l_rDynamicBoxContext.getInputChunkEndTime(i, j));
			}
		}
	}

	return true;
}