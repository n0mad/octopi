#pragma once
#include "./../model_abstract.h"
#include <vector>
#include <Eigen/Dense>
#include <ctime>

class TModelRNN : public TModel
{
public:
    const double LAMBDA = 0.85;
    const uint32 NORMALIZER = 1e6;//0x7FFFFFFF / 4;//1e6;

	TModelRNN(const std::string &fileName = "model_layer1.json");
    virtual void Encode(uint8 symbol, uint32 &low_count, uint32 &upper_count, uint32 &total);
    virtual uint8 Decode(uint32 value, uint32 &lower_count, uint32 &upper_count);
    virtual uint32 GetNormalizer() {
        return NORMALIZER;
    };
    virtual void Observe(uint8 symbol);
    virtual void Reset();
    virtual void UpdateSpace();
    void DumpSpace();
    void DumpState();
    virtual std::string GetName() {
        return "RNN";
    };
protected:
    Eigen::MatrixXd Softmax_B;
    Eigen::MatrixXd RnnBias;
    Eigen::VectorXd State;
    Eigen::VectorXd Space;

    Eigen::MatrixXd Embedding;
    Eigen::MatrixXd RnnW;
    Eigen::MatrixXd Softmax_W;
    Eigen::VectorXd Concat;

    std::vector<uint8> Chars;
};
