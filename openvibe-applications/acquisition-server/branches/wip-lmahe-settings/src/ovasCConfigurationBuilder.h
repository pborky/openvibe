#ifndef __OpenViBE_AcquisitionServer_CConfigurationBuilder_H__
#define __OpenViBE_AcquisitionServer_CConfigurationBuilder_H__

#include "ovas_base.h"

#include <gtk/gtk.h>

#include <string>
#include <map>

#include <sstream>

namespace OpenViBEAcquisitionServer
{
	class IHeader;

	class CConfigurationBuilder
	{
	public:

		CConfigurationBuilder(const char* sGtkBuilderFileName);
		virtual ~CConfigurationBuilder(void);

		virtual OpenViBE::boolean configure(
			OpenViBEAcquisitionServer::IHeader& rHeader);

		virtual void buttonChangeChannelNamesCB(void);
		virtual void buttonApplyChannelNameCB(void);
		virtual void buttonRemoveChannelNameCB(void);
		virtual void treeviewApplyChannelNameCB(void);

	protected:

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean doConfigure(void);
		virtual OpenViBE::boolean postConfigure(void);
		virtual OpenViBE::boolean saveConfiguration(void);
		virtual OpenViBE::boolean loadConfiguration(void);
		virtual OpenViBE::boolean checkLoad(void);

	private:

		CConfigurationBuilder(void);

	protected:

		OpenViBE::boolean m_bApplyConfiguration;

		::GtkBuilder* m_pBuilderConfigureInterface;
		::GtkBuilder* m_pBuilderConfigureChannelInterface;

		::GtkWidget* m_pDialog;

		::GtkWidget* m_pIdentifier;
		::GtkWidget* m_pAge;
		::GtkWidget* m_pNumberOfChannels;
		::GtkWidget* m_pSamplingFrequency;
		::GtkWidget* m_pGender;

		::GtkListStore* m_pElectrodeNameListStore;
		::GtkListStore* m_pChannelNameListStore;

		::GtkWidget* m_pElectrodeNameTreeView;
		::GtkWidget* m_pChannelNameTreeView;

		std::map < OpenViBE::uint32, std::string > m_vChannelName;
		std::string m_sGtkBuilderFileName;
		std::string m_sElectrodeFileName;
		std::string m_sGtkBuilderChannelsFileName;
		IHeader* m_pHeader;

		//#lm
		std::stringstream m_sChannelConfiguration;
		std::string m_sChannelConfigurationFilename;
		OpenViBE::boolean m_bLoadFromFile;// do we load the configuration from a file or not
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationBuilder_H__
