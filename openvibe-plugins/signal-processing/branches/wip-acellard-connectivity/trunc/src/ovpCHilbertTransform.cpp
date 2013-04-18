/*
 * HilbertTransform.cpp
 *
 *  Created on: Apr 12, 2013
 *      Author: ac-inria
 */

#if defined(TARGET_HAS_ThirdPartyEIGEN)

#include "ovpCAlgorithmPhaseLockingValue.h"
#include <complex>

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SignalProcessing;

using namespace Eigen;


boolean CAlgorithmHilbertTransform::initialize(void)
{
	ip_pMatrix.initialize(this->getInputParameter(OVP_Algorithm_HilbertTransform_InputParameterId_Matrix));
	op_pEnvelopeMatrix.initialize(this->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_EnvelopeMatrix));
	op_pPhaseMatrix.initialize(this->getOutputParameter(OVP_Algorithm_HilbertTransform_OutputParameterId_PhaseMatrix));

	return true;
}

boolean CAlgorithmHilbertTransform::uninitialize(void)
{
	op_pMatrix.uninitialize();
	ip_pMatrix.uninitialize();
	return true;
}

boolean CAlgorithmHilbertTransform::process(void)
{
	int l_ui32ChannelCount = ip_pMatrix->getDimensionSize(0);
	int l_ui32SamplesPerChannel = ip_pMatrix->getDimensionSize(1);

	IMatrix* l_pInputMatrix = ip_pMatrix;
	IMatrix* l_pOutputEnvelopeMatrix = op_pEnvelopeMatrix;
	IMatrix* l_pOutputPhaseMatrix = op_pPhaseMatrix;

	if(this->isInputTriggerActive(OVP_Algorithm_HilbertTransform_InputTriggerId_Process))
	{
		//Computing Hilbert transform for all channels
		for(uint32 channel=0; channel<l_ui32ChannelCount; channel++)
		{
			//Initialization of buffer vectors
			m_vecXcdSignalBuffer = RowVectorXcd::Zero(l_ui32SamplesPerChannel)
			m_vecXcdSignalFourier = RowVectorXcd::Zero(l_ui32SamplesPerChannel)

			//Initialization of vector h used to compute analytic signal
			m_vecXdHilbert.resize(l_ui32SamplesPerChannel);
			m_vecXdHilbert(0) = 1.0;
			if(l_ui32SamplesPerChannel%2 == 0)
			{
				m_vecXdHilbert(l_ui32SamplesPerChannel/2) = 1.0;
				for(unint32 i=1; i<l_ui32SamplesPerChannel/2; i++)
				{
					m_vecXdHilbert(i) = 2.0;
				}
				for(unint32 i=(l_ui32SamplesPerChannel/2)+1; i<l_ui32SamplesPerChannel; i++)
				{
					m_vecXdHilbert(i) = 0.0;
				}
			}
			else
			{
				m_vecXdHilbert((l_ui32SamplesPerChannel/2)+1) = 1.0;
				for(unint32 i=1; i<(l_ui32SamplesPerChannel/2)+1; i++)
				{
					m_vecXdHilbert(i) = 2.0;
				}
					for(unint32 i=(l_ui32SamplesPerChannel/2)+2; i<l_ui32SamplesPerChannel; i++)
				{
					m_vecXdHilbert(i) = 0.0;
				}
			}

			//Copy input signal chunk on buffer
			for(uint32 samples=0; samples<l_ui32SamplesPerChannel;samples++)
			{
				m_vecXcdSignalBuffer(samples).real(l_pInputMatrix->getBuffer()[samples + channel * (l_ui32SamplesPerChannel)]);
				m_vecXcdSignalBuffer(samples).imag(0.0);
			}

			//Fast Fourier Transform of input signal
			fwd(m_vecXcdSignalFourier, m_vecXcdSignalBuffer);

			//Apply Hilbert transform by element-wise multiplying fft vector by h
			m_vecXcdSignalFourier = m_vecXcdSignalFourier * m_vecXdHilbert;

			//Inverse Fast Fourier transform
			inv(m_vecXcdSignalBuffer, m_vecXcdSignalFourier); //m_vecXcdSignalBuffer is now the analytical signal of the initial input signal

			//Compute envelope and phase and pass it to the corresponding output
			for(uint32 samples=0; samples<l_ui32SamplesPerChannel;samples++)
			{
				l_pOutputEnvelopeMatrix->getBuffer()[samples + channel*samples] = abs(m_vecXcdSignalBuffer(samples));
				l_pOutputPhaseMatrix->getBuffer()[samples + channel*samples] = arg(m_vecXcdSignalBuffer(samples));
			}

		}
	}
}

#endif //TARGET_HAS_ThirdPartyEIGEN
