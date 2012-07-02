#include "ovpCBoxAlgorithmReferenceChannel.h"

#include <cstdio>
#include <cstdlib>



using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace OpenViBEToolkit;


namespace
{
	inline uint32 _find_channel_(const IMatrix& rMatrix, const CString& rChannel, const CIdentifier& rMatchMethodIdentifier, uint32 uiStart=0)
	{
		uint32 i, l_ui32Result=uint32(-1);

		if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Name)
		{
			for(i=uiStart; i<rMatrix.getDimensionSize(0); i++)
			{
				if(OpenViBEToolkit::Tools::String::isAlmostEqual(rMatrix.getDimensionLabel(0, i), rChannel, false))
				{
					l_ui32Result=i;
				}
			}
		}
		else if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Index)
		{
			unsigned int value;
			if(::sscanf(rChannel.toASCIIString(), "%u", &value)==1)
			{
				value--; // => makes it 0-indexed !
				if(uiStart <= uint32(value) && uint32(value) < rMatrix.getDimensionSize(0))
				{
					l_ui32Result=uint32(value);
				}
			}
		}
		else if(rMatchMethodIdentifier==OVP_TypeId_MatchMethod_Smart)
		{
			if(l_ui32Result==uint32(-1)) l_ui32Result=_find_channel_(rMatrix, rChannel, OVP_TypeId_MatchMethod_Name, uiStart);
			if(l_ui32Result==uint32(-1)) l_ui32Result=_find_channel_(rMatrix, rChannel, OVP_TypeId_MatchMethod_Index, uiStart);
		}

		return l_ui32Result;
	}
};

boolean CBoxAlgorithmReferenceChannel::initialize(void)
{
	
	m_oDecoder.initialize(*this);
	m_pInputMatrix=m_oDecoder.getOutputMatrix();

	m_oEncoder.initialize(*this);
	m_oEncoder.getInputSamplingRate().setReferenceTarget(m_oDecoder.getOutputSamplingRate());
	m_pOutputMatrix=m_oEncoder.getInputMatrix();

	m_vLookup.clear();

	return true;
}

boolean CBoxAlgorithmReferenceChannel::uninitialize(void)
{
	m_oDecoder.uninitialize();
	m_oEncoder.uninitialize();
	return true;
}

boolean CBoxAlgorithmReferenceChannel::processInput(uint32 ui32InputIndex)
{
	this->getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();
	return true;
}

#include <cstdio>

boolean CBoxAlgorithmReferenceChannel::process(void)
{
	// IBox& l_rStaticBoxContext=this->getStaticBoxContext();
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();
	uint32 i, j, k;

	for(i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		m_oDecoder.decode(0, i);
		if(m_oDecoder.isHeaderReceived())
		{
			//IMatrix& l_rInputMatrix=*m_oDecoder.getOutputMatrix();
			//IMatrix& l_rOutputMatrix=*m_oEncoder.getInputMatrix();

			if(m_pInputMatrix->getDimensionSize(0)<2)
			{
				this->getLogManager() << LogLevel_Error << "Needs at least 2 channels, got " << m_pInputMatrix->getDimensionSize(0) << "\n";
				return false;
			}

			CString l_sChannel=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0);
			uint64 l_ui64MatchMethod=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1);
			
			
			// ______________________________________________________________________________________________________________________________________________________
			//
			// Splits the channel list in order to build up the look up table
			// The look up table is later used to fill in the matrix content
			// ______________________________________________________________________________________________________________________________________________________
			//

			std::vector < CString > l_sToken;
			uint32 l_ui32TokenCount=OpenViBEToolkit::Tools::String::split(l_sChannel, OpenViBEToolkit::Tools::String::TSplitCallback < std::vector < CString > >(l_sToken), OV_Value_EnumeratedStringSeparator);


			for(j=0; j<l_ui32TokenCount; j++)
			{
				std::vector < CString > l_sSubToken;

				// Checks if the token is a range
				if(OpenViBEToolkit::Tools::String::split(l_sToken[j], OpenViBEToolkit::Tools::String::TSplitCallback < std::vector < CString > >(l_sSubToken), OV_Value_RangeStringSeparator)==2)
				{
					// Finds the first & second part of the range (only index based)
					uint32 l_ui32RangeStartIndex=::_find_channel_(*m_pInputMatrix, l_sSubToken[0], OVP_TypeId_MatchMethod_Index);
					uint32 l_ui32RangeEndIndex=::_find_channel_(*m_pInputMatrix, l_sSubToken[1], OVP_TypeId_MatchMethod_Index);

					// When first or second part is not found but associated token is empty, don't consider this as an error
					if(l_ui32RangeStartIndex==uint32(-1) && l_sSubToken[0]==CString("")) l_ui32RangeStartIndex=0;
					if(l_ui32RangeEndIndex  ==uint32(-1) && l_sSubToken[1]==CString("")) l_ui32RangeEndIndex=m_pInputMatrix->getDimensionSize(0)-1;

					 // After these corections, if either first or second token were not found, or if start index is greater than start index, consider this an error and invalid range
					if(l_ui32RangeStartIndex==uint32(-1) || l_ui32RangeEndIndex  ==uint32(-1) || l_ui32RangeStartIndex>l_ui32RangeEndIndex)
					{
						this->getLogManager() << LogLevel_Warning << "Invalid channel range [" << l_sToken[j] << "] - splitted as [" << l_sSubToken[0] << "][" << l_sSubToken[1] << "]\n";
					}
					else
					{
						// The range is valid so selects all the channels in this range
						this->getLogManager() << LogLevel_Trace << "For range [" << l_sToken[j] << "] :\n";
						for(k=l_ui32RangeStartIndex; k<=l_ui32RangeEndIndex; k++)
						{
							m_vLookup.push_back(k);
							this->getLogManager() << LogLevel_Trace << "  Selected channel [" << k+1 << "]\n";
						}
					}
				}
				else
				{
					// This is not a range, so we can consider the whole token as a single token name
					uint32 l_bFound=false;
					uint32 l_ui32Index=uint32(-1);

					// Looks for all the channels with this name
					while((l_ui32Index=::_find_channel_(*m_pInputMatrix, l_sToken[j], l_ui64MatchMethod, l_ui32Index+1))!=uint32(-1))
					{
						l_bFound=true;
						m_vLookup.push_back(l_ui32Index);
						this->getLogManager() << LogLevel_Trace << "Selected channel [" << l_ui32Index+1 << "]\n";
					}

					// When no channel was found, consider it a missing channel
					if(!l_bFound)
					{
						this->getLogManager() << LogLevel_Warning << "Invalid channel [" << l_sToken[j] << "]\n";
						m_vLookup.push_back(uint32(-1));
					}
				}
			}

			// ______________________________________________________________________________________________________________________________________________________
			//
			// Now we have the exact topology of the output matrix :)
			// ______________________________________________________________________________________________________________________________________________________
			//

			m_pOutputMatrix->setDimensionCount(2);
			//m_pOutputMatrix->setDimensionSize(0, m_pInputMatrix->getDimensionSize(0)-1);
			m_pOutputMatrix->setDimensionSize(0, m_pInputMatrix->getDimensionSize(0));
			m_pOutputMatrix->setDimensionSize(1, m_pInputMatrix->getDimensionSize(1));
			
			for(j=0, k=0; j<m_pInputMatrix->getDimensionSize(0); j++)
			{
				m_pOutputMatrix->setDimensionLabel(0, j, m_pInputMatrix->getDimensionLabel(0, j));
				/*for(uint32 jj=0; jj<m_vLookup.size(); jj++)
				{
					
					if(j!=m_vLookup[jj])
					{
						m_pOutputMatrix->setDimensionLabel(0, k++, m_pInputMatrix->getDimensionLabel(0, j));
					}	
				}*/
			}
			
			

			m_oEncoder.encodeHeader(0);
		}
		if(m_oDecoder.isBufferReceived())
		{
			/*IMatrix& l_rInputMatrix=*m_oDecoder.getOutputMatrix();
			IMatrix& l_rOutputMatrix=*m_oEncoder.getInputMatrix();
			float64* l_pInputBuffer=l_rInputMatrix.getBuffer();
			float64* l_pOutputBuffer=l_rOutputMatrix.getBuffer();
			uint32 l_ui32ChannelCount=l_rInputMatrix.getDimensionSize(0);
			uint32 l_ui32SampleCount=l_rInputMatrix.getDimensionSize(1);*/
			
			float64* l_pInputBuffer=m_pInputMatrix->getBuffer();
			float64* l_pOutputBuffer=m_pOutputMatrix->getBuffer();
			uint32 l_ui32ChannelCount=m_pInputMatrix->getDimensionSize(0);
			uint32 l_ui32SampleCount=m_pOutputMatrix->getDimensionSize(1);
			
			
			
			
			


			
			float64*  l_pAveragedReferenceBuffer; 
			l_pAveragedReferenceBuffer = (float64 *)calloc(l_ui32SampleCount, sizeof(float64));


			for(uint32 iref=0; iref<m_vLookup.size(); iref++)
			{
				//float64* l_pReferenceBuffer=m_pInputMatrix->getBuffer()+m_vLookup[iref]*m_pInputMatrix->getDimensionSize(1);
				//float64* l_pReferenceBuffer=m_pInputMatrix->getBuffer();

				for(k=0; k<l_ui32SampleCount; k++)
				{
					l_pAveragedReferenceBuffer[k] = l_pAveragedReferenceBuffer[k] + ( (1.0/((float)(m_vLookup.size()))) * l_pInputBuffer[k+(m_vLookup[iref]*m_pInputMatrix->getDimensionSize(1))]);
					
				}	
			}
			
	
			
			
			for(j=0; j<l_ui32ChannelCount; j++)
			{			
				//float64* l_pInputBuffer=m_pInputMatrix->getBuffer()+j*m_pInputMatrix->getDimensionSize(1);

				for(k=0; k<l_ui32SampleCount; k++)
				{
					l_pOutputBuffer[k+(j*m_pInputMatrix->getDimensionSize(1))]=l_pInputBuffer[k+(j*m_pInputMatrix->getDimensionSize(1))]-l_pAveragedReferenceBuffer[k];

				}
				
				
				
				
				/*for(uint32 jj=0; jj<m_vLookup.size(); jj++)
				{
					for(k=0; k<l_ui32SampleCount; k++)
					{
						if(j!=m_vLookup[jj])
						{
							l_pOutputBuffer[k]=l_pInputBuffer[k]-l_pAveragedReferenceBuffer[k];
						}
						else
						{
							l_pOutputBuffer[k]=l_pInputBuffer[k];
						}
						}
						this->getLogManager() << LogLevel_Info << "  l_pOutputBuffer[k]    : "<<l_pOutputBuffer[k]<<"\n";	
					}*/
						//l_pOutputBuffer+=l_ui32SampleCount;
				
				//l_pInputBuffer+=l_ui32SampleCount;
			}
		free(l_pAveragedReferenceBuffer);

			m_oEncoder.encodeBuffer(0);
		}
		if(m_oDecoder.isEndReceived())
		{
			m_oEncoder.encodeEnd(0);
		}
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}

	return true;
}
