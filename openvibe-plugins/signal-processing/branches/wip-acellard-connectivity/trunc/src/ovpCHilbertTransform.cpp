/*
 * HilbertTransform.cpp
 *
 *  Created on: Apr 12, 2013
 *      Author: ac-inria
 */

#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCAlgorithmPhaseLockingValue.h"

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace Eigen;


boolean CAlgorithmHilbertTransform::initialize(void)
{
	ip_pMatrix.initialize(this->getInputParameter(OVP_Algorithm_HilbertTransform_InputParameterId_Matrix));
	op_pMatrix.initialize(this->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_Matrix));
	return true;
}

boolean CAlgorithmHilbertTransform::uninitialize(void)
{
	op_pMatrix.uninitialize();
	ip_pMatrix.uninitialize();
	return true;
}

boolean CAlgorithmHilbertTransform::process(OpenViBE::CString EnvelopeOrPhase)
{
	int l_ui32ChannelCount = ip_pMatrix->getDimensionSize(0);
	int l_ui32SamplesPerChannel = ip_pMatrix->getDimensionSize(1);

	IMatrix* l_pInputMatrix = ip_pMatrix;
	IMatrix* l_pOutputMatrix = op_pMatrix;

	if(this->isInputTriggerActive(OVP_Algorithm_HilbertTransform_InputTriggerId_Process))
	{
		//Computing Hilbert transform for all channels
		for(uint32 channel=0; channel<l_ui32ChannelCount; channel++)
		{
			//Initialization of buffer vectors
			m_vecXcdSignalBuffer = RowVectorXcd::Zero(l_ui32SamplesPerChannel)
			m_vecXcdSignalFourier = RowVectorXcd::Zero(l_ui32SamplesPerChannel)


			//Copy input signal chunk on buffer
			for(uint32 samples=0; samples<l_ui32SamplesPerChannel;samples++)
			{
				m_vecXcdSignalBuffer(samples).real(l_pInputMatrix->getBuffer()[samples + channel * (l_ui32SamplesPerChannel)]);
				m_vecXcdSignalBuffer(samples).imag(0.0);

				//Creation of vector h used to compute analytic
			}

			//Fast Fourier Transform of input signal
			m_vecXcdSignalFourier = FFT(m_vecXcdSignalBuffer);



		}
	}











}

/*

boolean CAlgorithmHilbertTransform::processEnvelope(void)
{

}

boolean CalgortihmHilbertTransform::processPhase(void)
{

}
*/

#endif //TARGET_HAS_ThirdPartyEIGEN

