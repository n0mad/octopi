#pragma once
#include "./../model_abstract.h"
#include <vector>
#include <Eigen/Dense>
#include <ctime>

class TModelRNN2 : public TModel
{
public:
    const double LAMBDA = 0.95;
    const uint32 NORMALIZER = 1e6;//0x7FFFFFFF / 4;//1e6;

	TModelRNN2(const std::string &fileName = "model_layer2.json");
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
        return "Two-layer RNN";
    };
protected:
    Eigen::MatrixXd Softmax_B;
    Eigen::MatrixXd RnnBias_0;
    Eigen::MatrixXd RnnBias_1;
    Eigen::VectorXd State_0;
    Eigen::VectorXd State_1;
    Eigen::VectorXd Space;

    Eigen::MatrixXd Embedding;
    Eigen::MatrixXd RnnW_0;
    Eigen::MatrixXd RnnW_1;
    Eigen::MatrixXd Softmax_W;
    Eigen::VectorXd Concat_0;
    Eigen::VectorXd Concat_1;

    std::vector<uint8> Chars;
};
