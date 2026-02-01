#pragma once

#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

namespace Statistics
{

    inline double mean(const std::vector<double> &data)
    {
        if (data.empty())
            return 0.0;
        return std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    }

    inline double variance(const std::vector<double> &data)
    {
        if (data.size() < 2)
            return 0.0;
        double m = mean(data);
        double sum = 0.0;
        for (double x : data)
        {
            sum += (x - m) * (x - m);
        }
        return sum / (data.size() - 1);
    }

    inline double stddev(const std::vector<double> &data)
    {
        return std::sqrt(variance(data));
    }

    inline double median(std::vector<double> data)
    {
        if (data.empty())
            return 0.0;
        std::sort(data.begin(), data.end());
        size_t n = data.size();
        if (n % 2 == 0)
        {
            return (data[n / 2 - 1] + data[n / 2]) / 2.0;
        }
        return data[n / 2];
    }

    inline double percentile(std::vector<double> data, double p)
    {
        if (data.empty())
            return 0.0;
        std::sort(data.begin(), data.end());
        double idx = (p / 100.0) * (data.size() - 1);
        size_t lower = static_cast<size_t>(idx);
        size_t upper = lower + 1;
        if (upper >= data.size())
            return data.back();
        double frac = idx - lower;
        return data[lower] * (1.0 - frac) + data[upper] * frac;
    }

    inline double giniCoefficient(std::vector<double> data)
    {
        if (data.empty())
            return 0.0;
        std::sort(data.begin(), data.end());
        double n = static_cast<double>(data.size());
        double sum = 0.0;
        double cumSum = 0.0;
        for (size_t i = 0; i < data.size(); ++i)
        {
            cumSum += data[i];
            sum += (2.0 * (i + 1) - n - 1) * data[i];
        }
        if (cumSum == 0.0)
            return 0.0;
        return sum / (n * cumSum);
    }

    // Random number generator singleton
    class Random
    {
    public:
        static Random &getInstance()
        {
            static Random instance;
            return instance;
        }

        void seed(unsigned int s)
        {
            m_gen.seed(s);
        }

        // Uniform [0, 1)
        double uniform()
        {
            return m_dist(m_gen);
        }

        // Uniform [min, max)
        double uniform(double min, double max)
        {
            return min + (max - min) * uniform();
        }

        // Integer [min, max]
        int uniformInt(int min, int max)
        {
            std::uniform_int_distribution<int> dist(min, max);
            return dist(m_gen);
        }

        // Normal distribution
        double normal(double mean, double stddev)
        {
            std::normal_distribution<double> dist(mean, stddev);
            return dist(m_gen);
        }

        // Exponential distribution
        double exponential(double lambda)
        {
            std::exponential_distribution<double> dist(lambda);
            return dist(m_gen);
        }

        // Bernoulli (coin flip with probability p)
        bool bernoulli(double p)
        {
            return uniform() < p;
        }

    private:
        Random() : m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

        std::mt19937 m_gen;
        std::uniform_real_distribution<double> m_dist;
    };
}
