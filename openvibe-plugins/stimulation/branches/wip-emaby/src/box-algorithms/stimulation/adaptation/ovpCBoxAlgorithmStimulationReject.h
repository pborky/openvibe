#ifndef __OpenViBEPlugins_BoxAlgorithm_StimulationReject_H__
#define __OpenViBEPlugins_BoxAlgorithm_StimulationReject_H__

#include "../../../ovp_defines.h"
#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>
#include <vector>

// TODO:
// - please move the identifier definitions in ovp_defines.h
// - please include your desciptor in ovp_main.cpp

#define OVP_ClassId_BoxAlgorithm_StimulationReject     OpenViBE::CIdentifier(0xC7249E43, 0xDC357348)
#define OVP_ClassId_BoxAlgorithm_StimulationRejectDesc OpenViBE::CIdentifier(0x439C9F52, 0x48FFDA88)
#define OVP_TypeId_StimulationRejectAction             OpenViBE::CIdentifier(0x7A0D7546, 0xE4521837)
#define OVP_TypeId_StimulationRejectAction_Select      OpenViBE::CIdentifier(0xEA4B8DD9, 0xB7E7201C)
#define OVP_TypeId_StimulationRejectAction_Reject       OpenViBE::CIdentifier(0x3D1C1627, 0x1567D510)




namespace OpenViBEPlugins
{
	namespace Stimulation
	{
		class CBoxAlgorithmStimulationReject : public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:

			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_StimulationReject);

		protected:

			OpenViBE::Kernel::IAlgorithmProxy* m_pStimulationDecoder;
			OpenViBE::Kernel::IAlgorithmProxy* m_pStimulationEncoder;
			OpenViBE::Kernel::TParameterHandler < const OpenViBE::IMemoryBuffer* > ip_pMemoryBuffer;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* > op_pStimulationSet;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IStimulationSet* > ip_pStimulationSet;
			OpenViBE::Kernel::TParameterHandler < OpenViBE::IMemoryBuffer* > op_pMemoryBuffer;

			typedef struct
			{
				OpenViBE::uint64 ui64StartStimulationId;
				OpenViBE::uint64 ui64EndStimulationId;
			} SRule;

			OpenViBE::uint64 m_ui64DefaultAction;
			OpenViBE::uint64 m_ui64Action;
			OpenViBEPlugins::Stimulation::CBoxAlgorithmStimulationReject::SRule m_vRules;
			
			
			//std::vector < OpenViBEPlugins::Stimulation::CBoxAlgorithmStimulationReject::SRule > m_vRules;
		};

		class CBoxAlgorithmStimulationRejectListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{

		public:
			

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};

		class CBoxAlgorithmStimulationRejectDesc : public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Stimulation Reject"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Manu Maby"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("INSERM 1028"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Rejects incoming stimulations in time range defined by 2 stimulations"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString(""); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Stimulation/Adaptation"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-missing-image"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_StimulationReject; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::Stimulation::CBoxAlgorithmStimulationReject; }

			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput  ("Stimulations",            OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addOutput ("Modified Stimulations",   OV_TypeId_Stimulations);
				rBoxAlgorithmPrototype.addSetting("Begin Time Stimulation ", OV_TypeId_Stimulation, "OVTK_StimulationId_Label_00");
				rBoxAlgorithmPrototype.addSetting("End Time Stimulation ",   OV_TypeId_Stimulation, "OVTK_StimulationId_Label_0F");

				// rBoxAlgorithmPrototype.addFlag   (OpenViBE::Kernel::BoxFlag_IsUnstable);
				return true;
			}

			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const { return new CBoxAlgorithmStimulationRejectListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) { delete pBoxListener; }

			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_StimulationRejectDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_StimulationReject_H__
