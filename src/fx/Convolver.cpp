#include "fx/Convolver.h"

#include <math.h>
#include <algorithm>

#define MAX_NUM_THREADS 2

AUD_NAMESPACE_BEGIN
Convolver::Convolver(std::shared_ptr<std::vector<std::shared_ptr<std::vector<fftwf_complex>>>> ir, int N, int irLength, bool measure) :
	m_M(N / 2), m_L(N / 2), m_N(N), m_irBuffers(ir), m_irLength(irLength), m_inLength(0), m_readPosition(0), m_writePosition(0)
{
	m_resetFlag = false;
	for (int i = 0; i < m_irBuffers->size(); i++)
	{
		m_fftConvolvers.push_back(std::make_unique<FFTConvolver>((*m_irBuffers)[i], m_M, m_L, m_N, measure));
		m_fftOutBuffers.push_back((sample_t*)std::malloc(m_L * sizeof(sample_t)));
	}	

	m_bufLength = std::ceil((float)irLength / (float)m_L)*m_L * 2;
	m_outBuffer = (sample_t*)std::calloc(m_bufLength, sizeof(sample_t));
	m_inBuffer = (sample_t*)std::malloc(m_L*sizeof(sample_t));
	m_numThreads = std::min(MAX_NUM_THREADS, (int)m_irBuffers->size() - 1);
	m_threads = std::vector<std::thread>(m_numThreads);
}

Convolver::~Convolver()
{
	std::free(m_outBuffer);
	for (auto buf : m_fftOutBuffers)
		std::free(buf);
}

void Convolver::getNext(sample_t* buffer, int& length)
{
	if (length > m_L)
	{
		length = 0;
		return;
	}

	m_fftConvolvers[0]->getNext(buffer, m_fftOutBuffers[0], length);
	for (int i = 0; i < m_threads.size(); i++)
	{
		if (m_threads[i].joinable())
			m_threads[i].join();
	}
	
	m_writePosition += m_inLength;
	if (m_writePosition > m_bufLength - length)
	{
		m_writePosition = 0;
		std::memset(m_outBuffer + (m_bufLength / 2), 0, (m_bufLength / 2)*sizeof(sample_t));
	}
	else
		if (m_writePosition == m_bufLength / 2)
			std::memset(m_outBuffer, 0, (m_bufLength / 2)*sizeof(sample_t));

	m_inLength = length;
	std::memcpy(m_inBuffer, buffer, length*sizeof(sample_t));
	for (int i = 0; i < m_threads.size(); i++)
		m_threads[i] = std::thread(&Convolver::threadFunction, this, i);

	for (int i = 0; i < length; i++)
		m_outBuffer[i + m_writePosition] += m_fftOutBuffers[0][i];

	int nElem = length;
	m_readPosition = m_writePosition;
	std::memcpy(buffer, m_outBuffer + m_readPosition, nElem*sizeof(sample_t));
}

void Convolver::getRest(sample_t* buffer, int& length)
{

}

void Convolver::reset()
{
	m_mutex.lock();
	m_resetFlag = true;
	m_inLength = 0;
	m_readPosition = 0;
	m_writePosition = 0;
	std::memcpy(m_outBuffer, 0, m_bufLength*sizeof(sample_t));
	m_mutex.unlock();
}

void Convolver::threadFunction(int id)
{
	int total = m_irBuffers->size();
	int share = std::ceil(((float)total - 1) / (float)m_numThreads);
	int start = id*share+1;
	int end = std::min(start + share, total);
	
	for (int i = start; i < end; i++)
	{
		m_fftConvolvers[i]->getNext(m_inBuffer, m_fftOutBuffers[id+1], m_inLength);
		//m_mutex.lock();
		if (m_resetFlag) 
		{
			m_resetFlag = false;
			i = end;
		}
		else
		{
			int delay = i*m_M;
			int position = 0;
			for (int j = 0; j < m_inLength; j++)
			{
				position = j + m_writePosition + delay;
				if (position >= m_bufLength)
					position -= m_bufLength;
				m_outBuffer[position] += m_fftOutBuffers[id+1][j];
			}
		}
		//m_mutex.unlock();
	}
}
AUD_NAMESPACE_END