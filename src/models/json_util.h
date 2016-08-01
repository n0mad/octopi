#pragma once
#include "./../picojson/picojson.h"
#include <Eigen/Dense>

namespace OctopiJsonHelper {
    inline void ParseVector(Eigen::MatrixXd &v, const picojson::value &o) {
        const picojson::value::array &b = o.get<picojson::value::array>();
        for(int j = 0; j < b.size(); ++j) {
            double d = b[j].get<double>();
            v(j) = d;
        };
    };

    inline static void ParseMatrix(Eigen::MatrixXd &m, const picojson::value &o) {
        const picojson::value::array &b = o.get<picojson::value::array>();
        for(int i = 0; i < b.size(); ++i) {
            const picojson::value::array &row = b[i].get<picojson::value::array>();
            for(int j = 0; j < row.size(); ++j) {
                double d = row[j].get<double>();
                m(i, j) = d;
            };
        };
    };

}
