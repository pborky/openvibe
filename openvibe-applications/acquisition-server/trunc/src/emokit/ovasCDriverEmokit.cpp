#if defined TARGET_HAS_ThirdPartyEmokit

#include "ovasCDriverEmokit.h"
#include "ovasCConfigurationEmokit.h"

#include <system/Time.h>

#include <system/Memory.h>

#include <emokit/emokit.h>

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <vector>

using namespace OpenViBEAcquisitionServer;
using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace std;

#define boolean OpenViBE::boolean

CDriverEmokit::CDriverEmokit(IDriverContext& rDriverContext)
	:IDriver(rDriverContext)
	,m_pCallback(NULL)
	,m_pSample(NULL)
	,m_device(NULL)
{
	m_bUseGyroscope = false;
	m_ui32UserID = 0;
}

CDriverEmokit::~CDriverEmokit(void)
{
}

const char* CDriverEmokit::getName(void)
{
	return "Emokit";
}

//___________________________________________________________________//
//                                                                   //
boolean CDriverEmokit::initialize(
	const uint32 ui32SampleCountPerSentBlock,
	IDriverCallback& rCallback)
{
	if(m_bUseGyroscope)
	{
		m_oHeader.setChannelCount(16); // 14 + 2 Gyro 
	}
	else
	{
		m_oHeader.setChannelCount(14);
	}
	
	m_oHeader.setChannelName(0,  "AF3");
	m_oHeader.setChannelName(1,  "F7");
	m_oHeader.setChannelName(2,  "F3");
	m_oHeader.setChannelName(3,  "FC5");
	m_oHeader.setChannelName(4,  "T7");
	m_oHeader.setChannelName(5,  "P7");
	m_oHeader.setChannelName(6,  "O1");
	m_oHeader.setChannelName(7,  "O2");
	m_oHeader.setChannelName(8,  "P8");
	m_oHeader.setChannelName(9,  "T8");
	m_oHeader.setChannelName(10, "FC6");
	m_oHeader.setChannelName(11, "F4");
	m_oHeader.setChannelName(12, "F8");
	m_oHeader.setChannelName(13, "AF4");

	if(m_bUseGyroscope)
	{
		m_oHeader.setChannelName(14, "Gyro-X");
		m_oHeader.setChannelName(15, "Gyro-Y");
	}

	m_oHeader.setSamplingFrequency(128);

	m_rDriverContext.getLogManager() << LogLevel_Trace << "INIT called.\n";
	if(m_rDriverContext.isConnected())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] Emokit Driver: Driver already initialized.\n";
		return false;
	}

	if(!m_oHeader.isChannelCountSet()
	 ||!m_oHeader.isSamplingFrequencySet())
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] Emokit Driver: Channel count or frequency not set.\n";
		return false;
	}

	//---------------------------------------------------------
	// Builds up a buffer to store acquired samples. This buffer will be sent to the acquisition server later.

	m_pSample=new float32[m_oHeader.getChannelCount()];
	if( !m_pSample )
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] Emokit Driver: Samples allocation failed.\n";
		return false;
	}

	//__________________________________
	// Hardware initialization

	m_device = emokit_create();
	
	if( !m_device )
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] Emokit: Failed to create emokit object\n";
		return false;
	}
	
	uint32_t nDevices = emokit_get_count( m_device, EMOKIT_VID, EMOKIT_PID );
	m_rDriverContext.getLogManager() << LogLevel_Info << "[INIT] Emokit: " << nDevices << " devices connected.\n";
	
	if( m_ui32UserID >= nDevices )
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] Emokit: There is no headset with ID " << m_ui32UserID << "\n";
		emokit_delete( m_device );
		m_device = NULL;
		return false;
	}
	
	if( emokit_open( m_device, EMOKIT_VID, EMOKIT_PID, m_ui32UserID ) != 0 )
	{
		m_rDriverContext.getLogManager() << LogLevel_Error << "[INIT] Emokit: Could not connect to headset " << m_ui32UserID << "\n";
		emokit_delete( m_device );
		m_device = NULL;
		return false;
	}
	

	//__________________________________
	// Save parameters

	m_pCallback=&rCallback;
	return true;
}

boolean CDriverEmokit::start(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "START called.\n";
	
	if( !m_device )
	{
		return false;
	}
	
	if(!m_rDriverContext.isConnected())
	{
		return false;
	}

	if(m_rDriverContext.isStarted())
	{
		return false;
	}

	return true;
}

double CDriverEmokit::qual(short value)
{
	return 10000. - 1.25 * ( (double) value  ) ;
}

double CDriverEmokit::sign(int value)
{	
	return (double) value ;
}

boolean CDriverEmokit::loop(void)
{
	if(!m_rDriverContext.isConnected())
	{
		return false;
	}

	if(m_rDriverContext.isStarted())
	{
		int read_result = emokit_read_data( m_device );
		if( read_result > 0 )
		{
			struct emokit_frame frame = emokit_get_next_frame( m_device );
			m_pSample[0] = sign( frame.AF3 );
			m_pSample[1] = sign( frame.F7 );
			m_pSample[2] = sign( frame.F3 );
			m_pSample[3] = sign( frame.FC5 );
			m_pSample[4] = sign( frame.T7 );
			m_pSample[5] = sign( frame.P7 );
			m_pSample[6] = sign( frame.O1 );
			m_pSample[7] = sign( frame.O2 );
			m_pSample[8] = sign( frame.P8 );
			m_pSample[9] = sign( frame.T8 );
			m_pSample[10] = sign( frame.FC6 );
			m_pSample[11] = sign( frame.F4 );
			m_pSample[12] = sign( frame.F8 );
			m_pSample[13] = sign( frame.AF4 );
			if(m_bUseGyroscope)
			{
				m_pSample[14] = sign( frame.gyroX );
				m_pSample[15] = sign( frame.gyroY );
			}
			m_pCallback->setSamples( m_pSample, 1 );
			m_rDriverContext.correctDriftSampleCount(m_rDriverContext.getSuggestedDriftCorrectionSampleCount());
			
		}
		else
		{
			m_rDriverContext.getLogManager() << LogLevel_Error << "[LOOP] emokit_read_data returned " << read_result << "\n";
		}
	
	}
	else
	{
		if (m_rDriverContext.isImpedanceCheckRequested())
		{
			int read_result = emokit_read_data( m_device );
			if( read_result > 0 )
			{
				struct emokit_frame frame = emokit_get_next_frame( m_device );				
				m_rDriverContext.updateImpedance(0, qual( frame.cq.AF3 ));
				m_rDriverContext.updateImpedance(1, qual( frame.cq.F7 ));
				m_rDriverContext.updateImpedance(2, qual( frame.cq.F3 ));
				m_rDriverContext.updateImpedance(3, qual( frame.cq.FC5 ));
				m_rDriverContext.updateImpedance(4, qual( frame.cq.T7 ));
				m_rDriverContext.updateImpedance(5, qual( frame.cq.P7 ));
				m_rDriverContext.updateImpedance(6, qual( frame.cq.O1 ));
				m_rDriverContext.updateImpedance(7, qual( frame.cq.O2 ));
				m_rDriverContext.updateImpedance(8, qual( frame.cq.P8 ));
				m_rDriverContext.updateImpedance(9, qual( frame.cq.T8 ));
				m_rDriverContext.updateImpedance(10, qual( frame.cq.FC6 ));
				m_rDriverContext.updateImpedance(11, qual( frame.cq.F4 ));
				m_rDriverContext.updateImpedance(12, qual( frame.cq.F8 ));
				m_rDriverContext.updateImpedance(13, qual( frame.cq.AF4 ));
			}
			else
			{
				m_rDriverContext.getLogManager() << LogLevel_Error << "[LOOP] emokit_read_data returned " << read_result << "\n";
			}
		}
	}
	return true;
}

boolean CDriverEmokit::stop(void)
{
	m_rDriverContext.getLogManager() << LogLevel_Trace << "STOP called.\n";

	if(!m_rDriverContext.isConnected())
	{
		return false;
	}

	if(!m_rDriverContext.isStarted())
	{
		return false;
	}
	return true;
}

boolean CDriverEmokit::uninitialize(void)
{
	if( m_device )
	{
		emokit_close( m_device );
		emokit_delete( m_device );
	}
	m_device = NULL;

	return true;
}

//___________________________________________________________________//
//                                                                   //

boolean CDriverEmokit::isConfigurable(void)
{
	return true;
}

boolean CDriverEmokit::configure(void)
{
    CConfigurationEmokit m_oConfiguration(m_rDriverContext, OpenViBE::Directories::getDataDir() + "/openvibe-applications/acquisition-server/interface-Emokit.ui", m_bUseGyroscope, m_ui32UserID);

	if(!m_oConfiguration.configure(m_oHeader)) 
	{
		return false;
	}

	return true;
}

#endif //TARGET_HAS_ThirdPartyEmokit
