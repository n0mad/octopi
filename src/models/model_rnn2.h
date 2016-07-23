#pragma once
#include "./../model_abstract.h"
#include <vector>
#include <Eigen/Dense>
#include <ctime>

class TModelRNN2 : public TModel
{
public:
    const double LAMBDA = 0.99;
    const uint32 NORMALIZER = 1e8;//0x7FFFFFFF / 4;//1e6;


	TModelRNN2(const std::string &fileName = "model.json");
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
        return "N-layer RNN";
    };
	~TModelRNN2() {};
protected:
    std::vector<uint8> Chars;

    Eigen::VectorXd Space;

    Eigen::MatrixXd Embedding;
    Eigen::MatrixXd Softmax_B;
    Eigen::MatrixXd Softmax_W;

    std::vector<Eigen::MatrixXd> Biases;
    std::vector<Eigen::MatrixXd> Matrixes;
    std::vector<Eigen::VectorXd> States;

    uint32 Observed;

};
