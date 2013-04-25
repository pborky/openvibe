//#if defined(TARGET_HAS_ThirdPartyEIGEN)

#ifndef __OpenViBEPlugins_BoxAlgorithm_ConnectivityMeasure_H__
#define __OpenViBEPlugins_BoxAlgorithm_ConnectivityMeasure_H__

#include "../../ovp_defines.h"

#include <openvibe/ov_all.h>
#include <openvibe-toolkit/ovtk_all.h>

// The unique identifiers for the box and its descriptor.
// Identifier are randomly chosen by the skeleton-generator.
//#define OVP_ClassId_BoxAlgorithm_ConnectivityMeasure OpenViBE::CIdentifier(0x8E3A1AEF, 0x7CACD368)
//#define OVP_ClassId_BoxAlgorithm_ConnectivityMeasureDesc OpenViBE::CIdentifier(0xA20B0A40, 0x1A92D645)

namespace OpenViBEPlugins
{
	namespace SignalProcessing
	{
		/**
		 * \class CBoxAlgorithmConnectivityMeasure
		 * \author Alison Cellard (Inria)
		 * \date Fri Apr 19 11:21:04 2013
		 * \brief The class CBoxAlgorithmConnectivityMeasure describes the box Connectivity Measure.
		 *
		 */
		class CBoxAlgorithmConnectivityMeasure : virtual public OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >
		{
		public:
			virtual void release(void) { delete this; }

			virtual OpenViBE::boolean initialize(void);
			virtual OpenViBE::boolean uninitialize(void);
				
			virtual OpenViBE::boolean processInput(OpenViBE::uint32 ui32InputIndex);
			virtual OpenViBE::boolean process(void);


			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxAlgorithm < OpenViBE::Plugins::IBoxAlgorithm >, OVP_ClassId_BoxAlgorithm_ConnectivityMeasure);

		protected:

			// Signal stream decoder
			OpenViBEToolkit::TSignalDecoder < CBoxAlgorithmConnectivityMeasure > m_oAlgo0_SignalDecoder;
			// Signal stream encoder
			OpenViBEToolkit::TSignalEncoder < CBoxAlgorithmConnectivityMeasure > m_oAlgo1_SignalEncoder;

			OpenViBE::Kernel::IAlgorithmProxy* m_pConnectivityMethod;



			//OpenViBE::Kernel::TParameterHandler < >

		};


		// If you need to implement a box Listener, here is a sekeleton for you.
		// Use only the callbacks you need.
		// For example, if your box has a variable number of input, but all of them must be stimulation inputs.
		// The following listener callback will ensure that any newly added input is stimulations :
		/*		
		virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
		{
			rBox.setInputType(ui32Index, OV_TypeId_Stimulations);
		};
		*/
		
		
		// The box listener can be used to call specific callbacks whenever the box structure changes : input added, name changed, etc.
		// Please uncomment below the callbacks you want to use.
		class CBoxAlgorithmConnectivityMeasureListener : public OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >
		{
		public:


			//virtual OpenViBE::boolean onInputConnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onInputDisconnected(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			virtual OpenViBE::boolean onInputAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index)
			{
				rBox.setInputType(ui32Index, OV_TypeId_Signal);
				return true;
			};
			virtual OpenViBE::boolean onInputRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onInputTypeChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onInputNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };

			//virtual OpenViBE::boolean onSettingAdded(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingRemoved(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingNameChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingDefaultValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };
			//virtual OpenViBE::boolean onSettingValueChanged(OpenViBE::Kernel::IBox& rBox, const OpenViBE::uint32 ui32Index) { return true; };

			_IsDerivedFromClass_Final_(OpenViBEToolkit::TBoxListener < OpenViBE::Plugins::IBoxListener >, OV_UndefinedIdentifier);
		};
		

		/**
		 * \class CBoxAlgorithmConnectivityMeasureDesc
		 * \author Alison Cellard (Inria)
		 * \date Fri Apr 19 11:21:04 2013
		 * \brief Descriptor of the box Connectivity Measure.
		 *
		 */
		class CBoxAlgorithmConnectivityMeasureDesc : virtual public OpenViBE::Plugins::IBoxAlgorithmDesc
		{
		public:

			virtual void release(void) { }

			virtual OpenViBE::CString getName(void) const                { return OpenViBE::CString("Connectivity Measure"); }
			virtual OpenViBE::CString getAuthorName(void) const          { return OpenViBE::CString("Alison Cellard"); }
			virtual OpenViBE::CString getAuthorCompanyName(void) const   { return OpenViBE::CString("Inria"); }
			virtual OpenViBE::CString getShortDescription(void) const    { return OpenViBE::CString("Measure connectivity between pairs of channel"); }
			virtual OpenViBE::CString getDetailedDescription(void) const { return OpenViBE::CString("Measure connectivity between pairs of channel using the kind of measure chosen (PLV, MSC, etc.)"); }
			virtual OpenViBE::CString getCategory(void) const            { return OpenViBE::CString("Signal processing/Connectivity"); }
			virtual OpenViBE::CString getVersion(void) const             { return OpenViBE::CString("1.0"); }
			virtual OpenViBE::CString getStockItemName(void) const       { return OpenViBE::CString("gtk-network"); }

			virtual OpenViBE::CIdentifier getCreatedClass(void) const    { return OVP_ClassId_BoxAlgorithm_ConnectivityMeasure; }
			virtual OpenViBE::Plugins::IPluginObject* create(void)       { return new OpenViBEPlugins::SignalProcessing::CBoxAlgorithmConnectivityMeasure; }
			
			
			virtual OpenViBE::Plugins::IBoxListener* createBoxListener(void) const               { return new CBoxAlgorithmConnectivityMeasureListener; }
			virtual void releaseBoxListener(OpenViBE::Plugins::IBoxListener* pBoxListener) const { delete pBoxListener; }
			
			virtual OpenViBE::boolean getBoxPrototype(
				OpenViBE::Kernel::IBoxProto& rBoxAlgorithmPrototype) const
			{
				rBoxAlgorithmPrototype.addInput("EEG Signal",OV_TypeId_Signal);
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddInput);
//				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanModifyInput);
				
				rBoxAlgorithmPrototype.addOutput("Connectivity measure",OV_TypeId_Signal);
				
				rBoxAlgorithmPrototype.addSetting("Method",OVP_TypeId_ConnectivityAlgorithm, OVP_TypeId_ConnectivityAlgorithm_SPhaseLockingValue.toString());
				rBoxAlgorithmPrototype.addSetting("Pairs of channels",OV_TypeId_String,"1-2:3-4");
				rBoxAlgorithmPrototype.addSetting("Channel Matching Method",  OVP_TypeId_MatchMethod, OVP_TypeId_MatchMethod_Smart.toString());

//				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_CanAddSetting);
				
				rBoxAlgorithmPrototype.addFlag(OpenViBE::Kernel::BoxFlag_IsUnstable);
				
				return true;
			}
			_IsDerivedFromClass_Final_(OpenViBE::Plugins::IBoxAlgorithmDesc, OVP_ClassId_BoxAlgorithm_ConnectivityMeasureDesc);
		};
	};
};

#endif // __OpenViBEPlugins_BoxAlgorithm_ConnectivityMeasure_H__
//#endif //TARGET_HAS_ThirdPartyEIGEN
