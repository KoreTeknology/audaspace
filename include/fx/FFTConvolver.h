#pragma once

/**
* @file FFTConvolver.h
* @ingroup fx
* The FFTConvolver class.
*/

#include "IReader.h"
#include "ISound.h"
#include "util/FFTPlan.h"

#include <memory>
#include <vector>

AUD_NAMESPACE_BEGIN
/**
* This class allows to easily convolve a sound using the Fourier transform and the overlap add method.
*/
class AUD_API FFTConvolver
{
private:
	/**
	* A shared pointer to an FFT plan.
	*/
	std::shared_ptr<FFTPlan> m_plan;

	/**
	* The FFT size, must be at least M+L-1.
	*/
	int m_N;

	/**
	* The length of the impulse response.
	*/
	int m_M;

	/**
	* The max length of the input slices.
	*/
	int m_L;

	/**
	* The real length of the internal buffer in fftwf_complex elements.
	*/
	int m_realBufLen;

	/**
	* The internal buffer for the FFTS.
	*/
	void* m_inBuffer;

	/**
	* A shift buffer for the FDL method
	*/
	sample_t* m_shiftBuffer;

	/**
	* A buffer to store the extra data obtained after each partial convolution.
	*/
	float* m_tail;

	/**
	* The provided impulse response.
	*/
	std::shared_ptr<std::vector<fftwf_complex>> m_irBuffer;

	/**
	* If the tail is being read, this marks the current position.
	*/
	int m_tailPos;

	// delete copy constructor and operator=
	FFTConvolver(const FFTConvolver&) = delete;
	FFTConvolver& operator=(const FFTConvolver&) = delete;

public:
	/**
	* Creates a new FFTConvolver. This constructor uses the default value of N (FIXED_N constant)
	* \param ir A shared pointer to a vector with the impulse response data in the frequency domain (see ImpulseResponse class for an easy way to obtain it).
	* \param plan A shared pointer to and FFT plan.
	*/
	FFTConvolver(std::shared_ptr<std::vector<fftwf_complex>> ir, std::shared_ptr<FFTPlan> plan);
	virtual ~FFTConvolver();

	/**
	* Convolves the data that is provided with the inpulse response
	* \param[in] inBuffer A buffer with the input data to be convolved.
	* \param[in] outBuffer A pointer to the buffer in which the convolution result will be written.
	* \param[in,out] length The number of samples to be convolved (the length of both the inBuffer and the outBuffer).
	*						The convolution output should be larger than the input, but since this class uses the overlap
	*						add method, the extra length will be saved internally.
	*						It must be equal or lower than L or the call will fail, setting this variable to 0 since no data would be
	*						written in the outBuffer.
	*/
	void getNext(const sample_t* inBuffer, sample_t* outBuffer, int& length);

	/**
	* Convolves the data that is provided with the inpulse response
	* \param[in] inBufferA buffer with the input data to be convolved.
	* \param[in] outBuffer A pointer to the buffer in which the convolution result will be written.
	* \param[in,out] length The number of samples to be convolved (the length of both the inBuffer and the outBuffer).
	*						The convolution output should be larger than the input, but since this class uses the overlap
	*						add method, the extra length will be saved internally.
	*						It must be equal or lower than L or the call will fail, setting this variable to 0 since no data would be
	*						written in the outBuffer.
	* \param [in]transformedData A pointer to a buffer in which the Fourier transform of the input will be written.
	*/
	void getNext(const sample_t* inBuffer, sample_t* outBuffer, int& length, fftwf_complex* transformedData);

	/**
	* Convolves the data that is provided with the inpulse response
	* \param[in] inBuffer A buffer with the input data to be convolved. Its length must be N/2 + 1
	* \param[in] outBuffer A pointer to the buffer in which the convolution result will be written.
	* \param[in,out] length The number of samples to be convolved and the length of the outBuffer.
	*						The convolution output should be larger than the input, but since this class uses the overlap
	*						add method, the extra length will be saved internally.
	*						It must be equal or lower than L or the call will fail and set the value of length to 0 since no data would be
	*						written in the outBuffer.
	*/
	void getNext(const fftwf_complex* inBuffer, sample_t* outBuffer, int& length);
	
	/**
	* Gets the internally stored extra data which is result of the convolution.
	* \param[in,out] length The count of samples that should be read. Shall
	*                contain the real count of samples after reading, in case
	*                there were only fewer samples available.
	*                A smaller value also indicates the end of the data.
	* \param[out] eos End of stream, whether the end is reached or not.
	* \param[in] buffer The pointer to the buffer to read into.
	*/
	void getTail(int& length, bool& eos, sample_t* buffer);

	/**
	* Resets the internally stored data so a new convolution can be started.
	*/
	void clear();

	/**
	* Calculates the Inverse Fast Fourier Transform of the input array.
	* \param[in] inBuffer A buffer with the input data to be transformed. Its length must be N/2 + 1
	* \param[in] outBuffer A pointer to the buffer in which the transform result will be written. 
	* \param[in,out] length The number of samples to be transformed and the length of the outBuffer.
	*						It must be equal or lower than N or the call will fail and the value 
	*						of length will be setted to 0, since no data would be written in the outBuffer.
	*/
	void IFFT_FDL(const fftwf_complex* inBuffer, sample_t* outBuffer, int& length);

	/**
	* Multiplicates a frequency domain input by the impulse response and accumulates the result to a buffer.
	* \param[in] inBuffer A buffer of complex numbers, samples in the frequency domain, that will be multiplied by the impulse response. Its length must be N/2 + 1
	* \param[in] accBuffer A pointer to the buffer into which the result of the multiplication will be summed. Its length must be N/2 + 1
	*/
	void getNextFDL(const fftwf_complex* inBuffer, fftwf_complex* accBuffer);

	/**
	* Transforms an input array of real data to the frequency domain and multiplies it by the impulse response. The result is accumulated to a buffer.
	* \param[in] inBuffer A buffer of real numbers, samples in the time domain, that will be multiplied by the impulse response.
	* \param[in] accBuffer A pointer to the buffer into which the result of the multiplication will be summed. Its length must be N/2 + 1.
	* \param[in,out] length The number of samples to be transformed and the length of the inBuffer.
	*						It must be equal or lower than N/2 or the call will fail and the value
	*						of length will be setted to 0, since no data would be written in the outBuffer.
	* \param [in]transformedData A pointer to a buffer in which the Fourier transform of the input will be written.
	*/
	void getNextFDL(const sample_t* inBuffer, fftwf_complex* accBuffer, int& length, fftwf_complex* transformedData);
};

AUD_NAMESPACE_END
