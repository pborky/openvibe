#include "ovasCConfigurationBuilder.h"
#include "ovasIHeader.h"

#include <openvibe-toolkit/ovtk_all.h>

#include <iostream>
#include <fstream>
#include <list>
#include <cstdlib>

#define OVAS_ElectrodeNames_File           OpenViBE::Directories::getDataDir() + "/openvibe-applications/acquisition-server/electrode-names.txt"
#define OVAS_ConfigureGUIElectrodes_File   OpenViBE::Directories::getDataDir() + "/openvibe-applications/acquisition-server/interface-channel-names.ui"

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace std;

//___________________________________________________________________//
//                                                                   //

static void gtk_combo_box_set_active_text(::GtkComboBox* pComboBox, const gchar* sActiveText)
{
	::GtkTreeModel* l_pTreeModel=gtk_combo_box_get_model(pComboBox);
	::GtkTreeIter itComboEntry;
	int l_iIndex=0;
	gchar* l_sComboEntryName=NULL;
	if(gtk_tree_model_get_iter_first(l_pTreeModel, &itComboEntry))
	{
		do
		{
			gtk_tree_model_get(l_pTreeModel, &itComboEntry, 0, &l_sComboEntryName, -1);
			if(string(l_sComboEntryName)==string(sActiveText))
			{
				gtk_combo_box_set_active(pComboBox, l_iIndex);
				return;
			}
			else
			{
				l_iIndex++;
			}
		}
		while(gtk_tree_model_iter_next(l_pTreeModel, &itComboEntry));
	}
}

static void button_change_channel_names_cb(::GtkButton* pButton, void* pUserData)
{
#if defined _DEBUG_Callbacks_
	cout << "button_change_channel_names_cb" << endl;
#endif
	static_cast<CConfigurationBuilder*>(pUserData)->buttonChangeChannelNamesCB();
}

static void button_apply_channel_name_cb(::GtkButton* pButton, void* pUserData)
{
#if defined _DEBUG_Callbacks_
	cout << "button_apply_channel_name_cb" << endl;
#endif
	static_cast<CConfigurationBuilder*>(pUserData)->buttonApplyChannelNameCB();
}

static void button_remove_channel_name_cb(::GtkButton* pButton, void* pUserData)
{
#if defined _DEBUG_Callbacks_
	cout << "button_remove_channel_name_cb" << endl;
#endif
	static_cast<CConfigurationBuilder*>(pUserData)->buttonRemoveChannelNameCB();
}

static void treeview_apply_channel_name_cb(::GtkTreeView* pTreeview, ::GtkTreePath* pPath, ::GtkTreeViewColumn* pColumn, void* pUserData)
{
#if defined _DEBUG_Callbacks_
	cout << "treeview_apply_channel_name_cb" << endl;
#endif
	static_cast<CConfigurationBuilder*>(pUserData)->treeviewApplyChannelNameCB();
}

//___________________________________________________________________//
//                                                                   //

CConfigurationBuilder::CConfigurationBuilder(const char* sGtkBuilderFileName)
	:m_pBuilderConfigureInterface(NULL)
	,m_pElectrodeNameListStore(NULL)
	,m_pChannelNameListStore(NULL)
	,m_sGtkBuilderFileName(sGtkBuilderFileName?sGtkBuilderFileName:"")
	,m_sElectrodeFileName(OVAS_ElectrodeNames_File)
	,m_sGtkBuilderChannelsFileName(OVAS_ConfigureGUIElectrodes_File)
	,m_pHeader(NULL)
{
	std::string l_sDriver(USER_HOME);
	std::string l_sLoadMode(USER_HOME);
	std::string l_sAcquisition(USER_HOME);
	l_sDriver.append("/.config/openvibe/openvibe-acquisition-server-driverrc");
	l_sLoadMode.append("/.config/openvibe/openvibe-acquisition-loadmoderc");
	l_sAcquisition.append("/.config/openvibe/openvibe-acquisition-serverrc");

	m_sDriverConfigurationFilename = l_sDriver;
	m_sLoadModeConfigurationFilename = l_sLoadMode;
	m_sAcquisitionServerConfigurationFilename = l_sAcquisition;

	std::cout <<"m_sDriverConfigurationFilename "<<m_sDriverConfigurationFilename<<"\n";
	std::cout <<"m_sLoadModeConfigurationFilename "<<m_sLoadModeConfigurationFilename<<"\n";
}

CConfigurationBuilder::~CConfigurationBuilder(void)
{
}

//___________________________________________________________________//
//                                                                   //

boolean CConfigurationBuilder::configure(IHeader& rHeader)
{
	m_bApplyConfiguration=false;

	m_pHeader=&rHeader;
	this->preConfigure();
	m_bApplyConfiguration=this->doConfigure();
	this->postConfigure();
	m_pHeader=NULL;

	return m_bApplyConfiguration;
}

boolean CConfigurationBuilder::preConfigure(void)
{
	// Prepares interface

	m_pBuilderConfigureInterface=gtk_builder_new(); // glade_xml_new(m_sGtkBuilderFileName.c_str(), NULL, NULL);
	gtk_builder_add_from_file(m_pBuilderConfigureInterface, m_sGtkBuilderFileName.c_str(), NULL);

	m_pBuilderConfigureChannelInterface=gtk_builder_new(); // glade_xml_new(m_sGtkBuilderChannelsFileName.c_str(), NULL, NULL);
	gtk_builder_add_from_file(m_pBuilderConfigureChannelInterface, m_sGtkBuilderChannelsFileName.c_str(), NULL);

	// Finds all the widgets

	m_pDialog=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "openvibe-acquisition-server-settings"));

	m_pIdentifier=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_identifier"));
	m_pAge=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_age"));
	m_pNumberOfChannels=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "spinbutton_number_of_channels"));
	m_pSamplingFrequency=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_sampling_frequency"));
	m_pGender=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureInterface, "combobox_gender"));

	m_pElectrodeNameTreeView=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureChannelInterface, "treeview_electrode_names"));
	m_pChannelNameTreeView=GTK_WIDGET(gtk_builder_get_object(m_pBuilderConfigureChannelInterface, "treeview_channel_names"));

	// Prepares electrode name tree view

	::GtkTreeView* l_pElectrodeNameTreeView=GTK_TREE_VIEW(m_pElectrodeNameTreeView);
	::GtkCellRenderer* l_pElectrodeNameIndexCellRenderer=gtk_cell_renderer_text_new();
	::GtkTreeViewColumn* l_pElectrodeNameIndexTreeViewColumn=gtk_tree_view_column_new_with_attributes("Name", l_pElectrodeNameIndexCellRenderer, "text", 0, NULL);

	gtk_tree_view_append_column(l_pElectrodeNameTreeView, l_pElectrodeNameIndexTreeViewColumn);

	// Prepares channel name tree view

	::GtkTreeView* l_pChannelNameTreeView=GTK_TREE_VIEW(m_pChannelNameTreeView);
	::GtkCellRenderer* l_pChannelNameIndexCellRenderer=gtk_cell_renderer_text_new();
	::GtkCellRenderer* l_pChannelNameValueCellRenderer=gtk_cell_renderer_text_new();
	::GtkTreeViewColumn* l_pChannelNameIndexTreeViewColumn=gtk_tree_view_column_new_with_attributes("Index", l_pChannelNameIndexCellRenderer, "text", 0, NULL);
	::GtkTreeViewColumn* l_pChannelNameValueTreeViewColumn=gtk_tree_view_column_new_with_attributes("Name", l_pChannelNameValueCellRenderer, "text", 1, NULL);

	gtk_tree_view_append_column(l_pChannelNameTreeView, l_pChannelNameIndexTreeViewColumn);
	gtk_tree_view_append_column(l_pChannelNameTreeView, l_pChannelNameValueTreeViewColumn);

	// Connects custom GTK signals

	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureInterface,        "button_change_channel_names"), "pressed",       G_CALLBACK(button_change_channel_names_cb), this);
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureChannelInterface, "button_apply_channel_name"),   "pressed",       G_CALLBACK(button_apply_channel_name_cb),   this);
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureChannelInterface, "button_remove_channel_name"),  "pressed",       G_CALLBACK(button_remove_channel_name_cb),  this);
	g_signal_connect(gtk_builder_get_object(m_pBuilderConfigureChannelInterface, "treeview_electrode_names"),    "row-activated", G_CALLBACK(treeview_apply_channel_name_cb), this);
	gtk_builder_connect_signals(m_pBuilderConfigureInterface, NULL);
	gtk_builder_connect_signals(m_pBuilderConfigureChannelInterface, NULL);

	// Configures interface with preconfigured values

	if(m_pHeader->isExperimentIdentifierSet())
	{
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(m_pIdentifier),
			m_pHeader->getExperimentIdentifier());
	}
	if(m_pHeader->isSubjectAgeSet())
	{
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(m_pAge),
			m_pHeader->getSubjectAge());
	}
	if(m_pHeader->isChannelCountSet())
	{
		gtk_spin_button_set_value(
			GTK_SPIN_BUTTON(m_pNumberOfChannels),
			m_pHeader->getChannelCount());
	}
	if(m_pHeader->isSamplingFrequencySet())
	{
		char l_sSamplingFrequency[1024];
		sprintf(l_sSamplingFrequency, "%i", (int)m_pHeader->getSamplingFrequency());
		gtk_combo_box_set_active_text(
			GTK_COMBO_BOX(m_pSamplingFrequency),
			l_sSamplingFrequency);
	}
	else
	{
		gtk_combo_box_set_active(
			GTK_COMBO_BOX(m_pSamplingFrequency),
			0);
	}
	if(m_pHeader->isSubjectGenderSet())
	{
		gtk_combo_box_set_active_text(
			GTK_COMBO_BOX(m_pGender),
				m_pHeader->getSubjectGender()==OVTK_Value_Gender_Male?"male":
				m_pHeader->getSubjectGender()==OVTK_Value_Gender_Female?"female":
				m_pHeader->getSubjectGender()==OVTK_Value_Gender_Unknown?"unknown":
				"unspecified");
	}
	else
	{
		gtk_combo_box_set_active(
			GTK_COMBO_BOX(m_pGender),
			0);
	}

	// Prepares channel name cache
	if(m_pHeader->isChannelCountSet())
	{
		for(uint32 i=0; i<m_pHeader->getChannelCount(); i++)
		{
			m_vChannelName[i]=m_pHeader->getChannelName(i);
		}
	}

	return true;
}

boolean CConfigurationBuilder::doConfigure(void)
{
	//if we load the configuration from a file, we must not display the driver configuration dialog
	CheckLoad();
	if (m_bLoadFromFile)
	{
		load();
		return true;
	}
	else
		return gtk_dialog_run(GTK_DIALOG(m_pDialog))==GTK_RESPONSE_APPLY;
}

boolean CConfigurationBuilder::postConfigure(void)
{
	if(m_bApplyConfiguration)
	{
		gchar* l_sSamplingFrequency=gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pSamplingFrequency));
		string l_sGender=gtk_combo_box_get_active_text(GTK_COMBO_BOX(m_pGender));
		m_pHeader->setExperimentIdentifier(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pIdentifier)));
		m_pHeader->setSubjectAge(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pAge)));
		m_pHeader->setChannelCount(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pNumberOfChannels)));
		m_pHeader->setSamplingFrequency(l_sSamplingFrequency?atoi(l_sSamplingFrequency):0);
		m_pHeader->setSubjectGender(
			l_sGender=="male"?OVTK_Value_Gender_Male:
			l_sGender=="female"?OVTK_Value_Gender_Female:
			l_sGender=="unknown"?OVTK_Value_Gender_Unknown:
			OVTK_Value_Gender_NotSpecified);
		for(uint32 i=0; i!=m_pHeader->getChannelCount(); i++)
		{
			m_pHeader->setChannelName(i, m_vChannelName[i].c_str());
		}

		//Only save the configuration if we apply the changes and we did not load from a file
		if (!m_bLoadFromFile)
		{
		  save();
		}
	}

	gtk_widget_hide(m_pDialog);

	g_object_unref(m_pBuilderConfigureInterface);
	g_object_unref(m_pBuilderConfigureChannelInterface);
	m_pBuilderConfigureInterface=NULL;
	m_pBuilderConfigureChannelInterface=NULL;

	m_vChannelName.clear();

	return true;
}

void CConfigurationBuilder::buttonChangeChannelNamesCB(void)
{
	uint32 i;
	::GtkTreeIter itElectrodeName, itChannelName;
	::GtkDialog* l_pDialog=GTK_DIALOG(gtk_builder_get_object(m_pBuilderConfigureChannelInterface, "channel-names"));
	::GtkTreeView* l_pElectrodeNameTreeView=GTK_TREE_VIEW(m_pElectrodeNameTreeView);
	::GtkTreeView* l_pChannelNameTreeView=GTK_TREE_VIEW(m_pChannelNameTreeView);

	// Creates electrode name and channel name models

	m_pElectrodeNameListStore=gtk_list_store_new(1, G_TYPE_STRING);
	m_pChannelNameListStore=gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

	// Fills in electrode name model

	list<string> l_vElectrodeName;
	list<string>::iterator l;
	ifstream l_oFile(m_sElectrodeFileName.c_str());
	if(l_oFile.is_open())
	{
		while(!l_oFile.eof())
		{
			string l_sElectrodeName;
			l_oFile >> l_sElectrodeName;
			l_vElectrodeName.push_back(l_sElectrodeName);
		}
		l_oFile.close();

		for(l=l_vElectrodeName.begin(); l!=l_vElectrodeName.end(); l++)
		{
			gtk_list_store_append(m_pElectrodeNameListStore, &itElectrodeName);
			gtk_list_store_set(m_pElectrodeNameListStore, &itElectrodeName, 0, l->c_str(), -1);
		}
	}

	// Fills in channel name model

	uint32 l_ui32ChannelCount=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_pNumberOfChannels));
	for(i=0; i<l_ui32ChannelCount; i++)
	{
		char l_sChannelName[1024];
		sprintf(l_sChannelName, "%i", (int)(i+1));
		gtk_list_store_append(m_pChannelNameListStore, &itChannelName);
		gtk_list_store_set(m_pChannelNameListStore, &itChannelName, 0, l_sChannelName, -1);
		gtk_list_store_set(m_pChannelNameListStore, &itChannelName, 1, m_vChannelName[i].c_str(), -1);
	}

	// Attachs model to views

	gtk_tree_view_set_model(l_pElectrodeNameTreeView, GTK_TREE_MODEL(m_pElectrodeNameListStore));
	gtk_tree_view_set_model(l_pChannelNameTreeView, GTK_TREE_MODEL(m_pChannelNameListStore));

	// Selects first line of each
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pElectrodeNameListStore), &itElectrodeName))
	{
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(l_pElectrodeNameTreeView), &itElectrodeName);
	}
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
	{
		gtk_tree_selection_select_iter(gtk_tree_view_get_selection(l_pChannelNameTreeView), &itChannelName);
	}

	// Runs dialog !

	gint l_iDialogResponse;
	do
	{
		l_iDialogResponse=gtk_dialog_run(l_pDialog);
		switch(l_iDialogResponse)
		{
			case GTK_RESPONSE_APPLY:
				{
					//#lm reset the settings
					m_sChannelConfiguration.str("");
					m_sChannelConfiguration.clear();
					//Save the settings
					m_sChannelConfiguration << "#Channels name settings\n";
					std::stringstream l_sChannelConfigurationList;

					int i=0;
					gchar* l_sChannelName=NULL;
					if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
					{
						do
						{
							gtk_tree_model_get(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName, 1, &l_sChannelName, -1);
							m_vChannelName[i++]=l_sChannelName;
							l_sChannelConfigurationList << "Channel"<<i<<" = "<<l_sChannelName<<"\n";
						}
						while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName));
					}
					//#lm if we loaded the channels name from a file, save the file name
					//otherwise, save each channel name individually
					if (m_sChannelConfigurationFilename!="")
					{
					  std::cout <<"channel_configuration_filename = "<< m_sChannelConfigurationFilename <<"\n";
					  m_sChannelConfiguration <<"channel_configuration_filename = "<< m_sChannelConfigurationFilename <<"\n";
					}
					else
					{
					  std::cout << "channel list is\n " << l_sChannelConfigurationList.str();
					  m_sChannelConfiguration<<l_sChannelConfigurationList.str();
					}
				}
				break;

			case 1: // Load
				{
					::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
						"Select file to open...",
						NULL,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL);
/*
					gtk_file_chooser_set_current_folder(
						GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
						this->getWorkingDirectory().toASCIIString());
*/
					if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
					{
						char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));

						//get this filename for the configuration #lm
						m_sChannelConfigurationFilename = "";
						m_sChannelConfigurationFilename.append(l_sFileName);

						list<string> l_vElectrodeName;
						list<string>::iterator l;
						ifstream l_oFile(l_sFileName);
						if(l_oFile.is_open())
						{
							// Reads channel names from file
							while(!l_oFile.eof())
							{
								string l_sElectrodeName;
								l_oFile >> l_sElectrodeName;
								l_vElectrodeName.push_back(l_sElectrodeName);
							}
							l_oFile.close();

							// Clears list store
							if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
							{
								do
								{
									gtk_list_store_set(m_pChannelNameListStore, &itChannelName, 1, "", -1);
								}
								while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName));
							}

							// Fills list store with channel names
							l=l_vElectrodeName.begin();
							if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName) && l!=l_vElectrodeName.end())
							{
								do
								{
									gtk_list_store_set(m_pChannelNameListStore, &itChannelName, 1, l->c_str(), -1);
									l++;
								}
								while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName) && l!=l_vElectrodeName.end());
							}

						}
						g_free(l_sFileName);
					}
					gtk_widget_destroy(l_pWidgetDialogOpen);
				}
				break;

			case 2: // Save
				{
					::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
						"Select file to save to...",
						NULL,
						GTK_FILE_CHOOSER_ACTION_OPEN,
						GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
						GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
						NULL);
/*
					gtk_file_chooser_set_current_folder(
						GTK_FILE_CHOOSER(l_pWidgetDialogOpen),
						this->getWorkingDirectory().toASCIIString());
*/
					if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
					{
						char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
						ofstream l_oFile(l_sFileName);
						if(l_oFile.is_open())
						{
							gchar* l_sChannelName=NULL;
							if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
							{
								do
								{
									gtk_tree_model_get(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName, 1, &l_sChannelName, -1);
									l_oFile << l_sChannelName << "\n";
								}
								while(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName));
							}
						}
						g_free(l_sFileName);
					}
					gtk_widget_destroy(l_pWidgetDialogOpen);
				}
				break;
		}
	}
	while(l_iDialogResponse!=GTK_RESPONSE_APPLY && l_iDialogResponse!=GTK_RESPONSE_CANCEL);

	gtk_widget_hide(GTK_WIDGET(l_pDialog));
	g_object_unref(m_pChannelNameListStore);
	g_object_unref(m_pElectrodeNameListStore);
	m_pChannelNameListStore=NULL;
	m_pElectrodeNameListStore=NULL;
}

void CConfigurationBuilder::buttonApplyChannelNameCB(void)
{
	::GtkTreeIter itElectrodeName, itChannelName;
	::GtkTreeView* l_pElectrodeNameTreeView=GTK_TREE_VIEW(m_pElectrodeNameTreeView);
	::GtkTreeView* l_pChannelNameTreeView=GTK_TREE_VIEW(m_pChannelNameTreeView);

	::GtkTreeSelection* l_pChannelNameTreeViewSelection=gtk_tree_view_get_selection(l_pChannelNameTreeView);
	::GtkTreeSelection* l_pElectrodeNameTreeViewSelection=gtk_tree_view_get_selection(l_pElectrodeNameTreeView);

	if(!gtk_tree_selection_get_selected(l_pChannelNameTreeViewSelection, NULL, &itChannelName))
	{
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
		{
			gtk_tree_selection_select_iter(l_pChannelNameTreeViewSelection, &itChannelName);
		}
	}

	if(gtk_tree_selection_get_selected(l_pChannelNameTreeViewSelection, NULL, &itChannelName))
	{
		if(gtk_tree_selection_get_selected(l_pElectrodeNameTreeViewSelection, NULL, &itElectrodeName))
		{
			gchar* l_sElectrodeName=NULL;
			gtk_tree_model_get(GTK_TREE_MODEL(m_pElectrodeNameListStore), &itElectrodeName, 0, &l_sElectrodeName, -1);
			gtk_list_store_set(m_pChannelNameListStore, &itChannelName, 1, l_sElectrodeName, -1);
			if(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
			{
				gtk_tree_selection_select_iter(l_pChannelNameTreeViewSelection, &itChannelName);
			}
			else
			{
				if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
				{
					gtk_tree_selection_select_iter(l_pChannelNameTreeViewSelection, &itChannelName);
				}
			}
		}
	}
}

void CConfigurationBuilder::buttonRemoveChannelNameCB(void)
{
	::GtkTreeIter itChannelName;
	::GtkTreeView* l_pChannelNameTreeView=GTK_TREE_VIEW(m_pChannelNameTreeView);

	::GtkTreeSelection* l_pChannelNameTreeViewSelection=gtk_tree_view_get_selection(l_pChannelNameTreeView);
	if(gtk_tree_selection_get_selected(l_pChannelNameTreeViewSelection, NULL, &itChannelName))
	{
		gtk_list_store_set(m_pChannelNameListStore, &itChannelName, 1, "", -1);
		if(gtk_tree_model_iter_next(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
		{
			gtk_tree_selection_select_iter(l_pChannelNameTreeViewSelection, &itChannelName);
		}
		else
		{
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_pChannelNameListStore), &itChannelName))
			{
				gtk_tree_selection_select_iter(l_pChannelNameTreeViewSelection, &itChannelName);
			}
		}
	}
}

void CConfigurationBuilder::treeviewApplyChannelNameCB(void)
{
	this->buttonApplyChannelNameCB();
}

boolean CConfigurationBuilder::save(void)
{
	//std::cout <<" CConfigurationBuilder::save() \n";
	std::stringstream l_sConfigurationTokenAndValues;

	//retrieve all objects from the interface
	GSList * list = gtk_builder_get_objects(m_pBuilderConfigureInterface);
	for (int i=0; i<g_slist_length(list); i++)
	{
		//convention for naming these variables?
		gpointer pointer = g_slist_nth_data(list, i);
		GObject* object = (GObject*)pointer;
		GtkBuildable* buildable = (GtkBuildable*)object;
		const gchar* l_cObjectType = G_OBJECT_TYPE_NAME(object);
		string l_sObjectType(l_cObjectType);
		std::string Token;

		if (GTK_IS_BUILDABLE(buildable))
		{
			Token = gtk_buildable_get_name( buildable );

			if ( l_sObjectType == "GtkEntry" )
			{
				GtkEntry* Entry =  (GtkEntry*)object;
				l_sConfigurationTokenAndValues << Token << " = " << gtk_entry_get_text(Entry) <<"\n";
			}
			if ( l_sObjectType == "GtkSpinButton" )
			{
				GtkSpinButton* SpinButton =  (GtkSpinButton*)object;
				l_sConfigurationTokenAndValues << Token << " = " << gtk_spin_button_get_value(SpinButton) <<"\n";

			}
			if ( l_sObjectType == "GtkButton" )
			{
				GtkButton* Button =  (GtkButton*)object;
			}
			if ( l_sObjectType == "GtkToggleButton" )
			{
				GtkToggleButton* ToogleButton =  (GtkToggleButton*)object;
				l_sConfigurationTokenAndValues << Token << " = " << gtk_toggle_button_get_active(ToogleButton) <<"\n";
			}
			if ( l_sObjectType == "GtkComboBox" )
			{
				GtkComboBox* ComboBox =  (GtkComboBox*)object;
				l_sConfigurationTokenAndValues << Token << " = " << gtk_combo_box_get_active(ComboBox) <<"\n";
			}

		}
	}
	g_slist_free(list);


	//open and write in the driver configuration file
	OpenViBE::CString ConfFile(m_sDriverConfigurationFilename.c_str());
	FILE* l_pFile=::fopen(ConfFile, "wt");
	if(l_pFile)
	{
	::fprintf(l_pFile, "\n# Driver settings\n");
	::fprintf(l_pFile, l_sConfigurationTokenAndValues.str().c_str() );
	::fprintf(l_pFile, m_sChannelConfiguration.str().c_str() );
	::fclose(l_pFile);
	}
}

//this function check if we are loading a configuration from a file or not
//by checking in the load mode configuration file
//I do this because I do not have a mean to transmit a variable from the AcquisitionSeverGUI
//to this class without modifiying each driver
boolean CConfigurationBuilder::CheckLoad(void)
{
    OpenViBE::CString ConfFile(m_sLoadModeConfigurationFilename.c_str());
    FILE* l_pFile=::fopen(ConfFile, "r+");

    std::cout << "check loading  configuration from " << ConfFile <<"\n";
    uint32 mode;
    if(l_pFile)
    {
	  ::fscanf (l_pFile, "%u", &mode);
	  //::fseek ( l_pFile , 0 , SEEK_SET );
	  //::fprintf(l_pFile, "%u",1);
	  ::fclose(l_pFile);
    }
    std::cout <<"mode "<<mode<<"\n";

    m_bLoadFromFile = (mode==0)?false:true;
    return true;
}


boolean CConfigurationBuilder::load(void)
{
	OpenViBE::CString ConfFile(m_sAcquisitionServerConfigurationFilename.c_str());
	ifstream l_pFile;
	l_pFile.open(ConfFile);

	//std::cout << "CConfiguration builder loading  configuration from " << ConfFile <<"\n";

	std::map<std::string, std::string> l_mTokenValues;

	//get the token values
	if(l_pFile)
	{
		//TODO get to the driver section of the configuration file
		//and only read these tokens
		string sLine = "";
		while (!l_pFile.eof())
		{
			getline(l_pFile, sLine);

			int sep = sLine.find("=");
			if (sep!=-1)
			{
				string Token = sLine.substr(0,sep-1) ;
				string Value = sLine.substr(sep+2);
				l_mTokenValues[Token] = Value;
				//std::cout <<Token<<" = "<<Value<<"\n";
			}
		}
		l_pFile.close();
	}


	//put the values in
	//get all the objects
	GSList * list = gtk_builder_get_objects(m_pBuilderConfigureInterface);
	for (int i=0; i<g_slist_length(list); i++)
	{
		gpointer pointer = g_slist_nth_data(list, i);
		GObject* object = (GObject*)pointer;
		GtkBuildable* buildable = (GtkBuildable*)object;
		const gchar* l_cObjectType = G_OBJECT_TYPE_NAME(object);
		string l_sObjectType(l_cObjectType);
		std::string Token;
		std::map<std::string, std::string>::iterator Iterator;

		if (GTK_IS_BUILDABLE(buildable))
		{
			Token = gtk_buildable_get_name( buildable );
			//std::cout<<Token<<"\n";
			Iterator = l_mTokenValues.find(Token);
			//if the token correspond to a gtkWidget name
			//we fill this widget with the token value
			if ( Iterator!=l_mTokenValues.end() )
			{
				std::string Value = l_mTokenValues[Token];
				if ( l_sObjectType == "GtkEntry" )
				{
					GtkEntry* Entry =  (GtkEntry*)object;
					gtk_entry_set_text(Entry, Value.c_str() );
				}
				if ( l_sObjectType == "GtkSpinButton" )
				{
					GtkSpinButton* SpinButton =  (GtkSpinButton*)object;
					gtk_spin_button_set_value(SpinButton, atoi(Value.c_str()) );
				}
				if ( l_sObjectType == "GtkButton" )
				{
					GtkButton* Button =  (GtkButton*)object;
				}
				if ( l_sObjectType == "GtkToggleButton" )
				{
					GtkToggleButton* ToogleButton =  (GtkToggleButton*)object;
					gtk_toggle_button_set_active(ToogleButton, atoi(Value.c_str()) );
				}
				if ( l_sObjectType == "GtkComboBox" )
				{
					GtkComboBox* ComboBox =  (GtkComboBox*)object;
					gtk_combo_box_set_active(ComboBox, atoi(Value.c_str()) );
				}
			}
		}

	}
	g_slist_free(list);

	//channel names are treated differently, it can be either a filename or directly the list of the channels name
	std::map<std::string, std::string>::iterator Iterator;
	std::string Token = "channel_configuration_filename";
	Iterator = l_mTokenValues.find(Token);
	//if we have a filename
	if ( Iterator!=l_mTokenValues.end() )
	{
		std::string l_sFileName = l_mTokenValues[Token];
		//ChangeChannelNamesFromFile(l_sFilename.c_str());
		//same piece of code than in the function buttonChangeChannelNamesCB() switch case load
		//TODO : avoid code duplication
		std::list<string> l_vElectrodeName;
		std::list<string>::iterator l;
		ifstream l_oFile(l_sFileName.c_str());
		if(l_oFile.is_open())
		{
			// Reads channel names from file
			while(!l_oFile.eof())
			{
				string l_sElectrodeName;
				l_oFile >> l_sElectrodeName;
				l_vElectrodeName.push_back(l_sElectrodeName);
			}
			l_oFile.close();
		}

		int i=0;
		for (l = l_vElectrodeName.begin() ; l!=l_vElectrodeName.end(); l++)
		{
			m_vChannelName[i] = *l;
			i++;
		}
	}
	else //we have the list of the channels names
	{
		int i=1;
		do
		{
			std::stringstream Stream;
			Stream <<"Channel"<<i;
			Token = Stream.str();
			Iterator = l_mTokenValues.find(Token);
			std::string Value = l_mTokenValues[Token];
			m_vChannelName[i-1] = Value;
			i++;
		} while (Iterator!=l_mTokenValues.end());
	}


    m_bApplyConfiguration = true;


    //set the load mode back to 0 (not from file, display the configuration dialog)
    OpenViBE::CString ConfFile2(m_sLoadModeConfigurationFilename.c_str());
    FILE* l_pFile2=::fopen(ConfFile2, "wt");
    if(l_pFile2)
    {
	  ::fprintf(l_pFile2, "%u",0);
	  ::fclose(l_pFile2);
    }

}

