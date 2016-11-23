#pragma once
#include "./../model_abstract.h"
#include <vector>
#include <Eigen/Dense>
#include <ctime>

class TModelGRU : public TModel
{
public:
    const double LAMBDA = 0.950;
    const uint64 NORMALIZER = 1e10;

	TModelGRU(const std::string &fileName = "model.json");
    virtual void Encode(uint8 symbol, uint64 &low_count, uint64 &upper_count, uint64 &total);
    virtual uint8 Decode(uint64 value, uint64 &lower_count, uint64 &upper_count);
    virtual uint64 GetNormalizer() {
        return NORMALIZER;
    };
    virtual void Observe(uint8 symbol);
    virtual void Reset();
    virtual void UpdateSpace();
    void DumpSpace();
    void DumpState();
    virtual std::string GetName() {
        return "N-layer GRU";
    };
	~TModelGRU() {};
protected:
    std::vector<uint8> Chars;

    Eigen::VectorXd Space;

    Eigen::MatrixXd Embedding;
    Eigen::MatrixXd Softmax_B;
    Eigen::MatrixXd Softmax_W;

    std::vector<Eigen::MatrixXd> CandidateBiases;
    std::vector<Eigen::MatrixXd> CandidateMatrixes;
    std::vector<Eigen::MatrixXd> GateBiases;
    std::vector<Eigen::MatrixXd> GateMatrixes;

    std::vector<Eigen::VectorXd> States;
};
