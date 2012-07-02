#include "ovpCBoxAlgorithmStimulationReject.h"

#include <cstddef>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Stimulation;

boolean CBoxAlgorithmStimulationReject::initialize(void)
{
	IBox& l_rStaticBoxContext=this->getStaticBoxContext();

	m_ui64Action=OVP_TypeId_StimulationRejectAction_Select.toUInteger();

	m_vRules.ui64StartStimulationId=FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 0); // first stimulation
	m_vRules.ui64EndStimulationId  =FSettingValueAutoCast(*this->getBoxAlgorithmContext(), 1); // last stim in the desired range



	m_pStimulationEncoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamEncoder));
	m_pStimulationEncoder->initialize();

	m_pStimulationDecoder=&this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(OVP_GD_ClassId_Algorithm_StimulationStreamDecoder));
	m_pStimulationDecoder->initialize();

	ip_pMemoryBuffer.initialize(m_pStimulationDecoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_InputParameterId_MemoryBufferToDecode));
	op_pStimulationSet.initialize(m_pStimulationDecoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamDecoder_OutputParameterId_StimulationSet));
	ip_pStimulationSet.initialize(m_pStimulationEncoder->getInputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_InputParameterId_StimulationSet));
	op_pMemoryBuffer.initialize(m_pStimulationEncoder->getOutputParameter(OVP_GD_Algorithm_StimulationStreamEncoder_OutputParameterId_EncodedMemoryBuffer));

	return true;
}

boolean CBoxAlgorithmStimulationReject::uninitialize(void)
{
	op_pMemoryBuffer.uninitialize();
	ip_pStimulationSet.uninitialize();
	op_pStimulationSet.uninitialize();
	ip_pMemoryBuffer.uninitialize();

	m_pStimulationDecoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStimulationDecoder);

	m_pStimulationEncoder->uninitialize();
	this->getAlgorithmManager().releaseAlgorithm(*m_pStimulationEncoder);


	return true;
}

boolean CBoxAlgorithmStimulationReject::processInput(uint32 ui32InputIndex)
{
	getBoxAlgorithmContext()->markAlgorithmAsReadyToProcess();

	return true;
}

boolean CBoxAlgorithmStimulationReject::process(void)
{
	IBoxIO& l_rDynamicBoxContext=this->getDynamicBoxContext();

	for(uint32 i=0; i<l_rDynamicBoxContext.getInputChunkCount(0); i++)
	{
		ip_pMemoryBuffer=l_rDynamicBoxContext.getInputChunk(0, i);
		op_pMemoryBuffer=l_rDynamicBoxContext.getOutputChunk(0);
		m_pStimulationDecoder->process();
		if(m_pStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedHeader))
		{
			m_pStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeHeader);
		}
		if(m_pStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedBuffer))
		{
			ip_pStimulationSet->setStimulationCount(0);
			for(uint64 s=0; s<op_pStimulationSet->getStimulationCount(); s++)
			{
				uint64 l_ui64StimulationId=op_pStimulationSet->getStimulationIdentifier(s);
				uint64 l_ui64StimulationDate=op_pStimulationSet->getStimulationDate(s);
				
				
				if (l_ui64StimulationId == m_vRules.ui64StartStimulationId)
				{
					m_ui64Action = OVP_TypeId_StimulationRejectAction_Reject.toUInteger();
				}
				
				if (l_ui64StimulationId == m_vRules.ui64EndStimulationId)
				{
					m_ui64Action = OVP_TypeId_StimulationRejectAction_Select.toUInteger();
				}
				
				
				if(m_ui64Action==OVP_TypeId_StimulationRejectAction_Select.toUInteger())
				{
					this->getLogManager() << LogLevel_Trace << "Selects stimulation " << this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, l_ui64StimulationId) << " !\n";
					ip_pStimulationSet->appendStimulation(
						l_ui64StimulationId,
						l_ui64StimulationDate,
						op_pStimulationSet->getStimulationDuration(s));

				}
				if(m_ui64Action==OVP_TypeId_StimulationRejectAction_Reject.toUInteger())
				{
					this->getLogManager() << LogLevel_Trace << "Rejects stimulation " << this->getTypeManager().getEnumerationEntryNameFromValue(OV_TypeId_Stimulation, l_ui64StimulationId) << " !\n";
				}
			}

			m_pStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeBuffer);
		}
		if(m_pStimulationDecoder->isOutputTriggerActive(OVP_GD_Algorithm_StimulationStreamDecoder_OutputTriggerId_ReceivedEnd))
		{
			m_pStimulationEncoder->process(OVP_GD_Algorithm_StimulationStreamEncoder_InputTriggerId_EncodeEnd);
		}
		l_rDynamicBoxContext.markInputAsDeprecated(0, i);
		l_rDynamicBoxContext.markOutputAsReadyToSend(0, l_rDynamicBoxContext.getInputChunkStartTime(0, i), l_rDynamicBoxContext.getInputChunkEndTime(0, i));
	}

	return true;
}
