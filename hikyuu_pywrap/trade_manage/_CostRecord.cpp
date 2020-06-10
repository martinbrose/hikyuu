/*
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2020-6-10
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <hikyuu/trade_manage/CostRecord.h>
#include "../pickle_support.h"

using namespace hku;
namespace py = pybind11;

std::string CostRecord_to_string(const CostRecord& record) {
    return fmt::format("{}", record);
}

void export_CostRecord(py::module& m) {
    py::class_<CostRecord>(m, "CostRecord", R"(成本记录
    
    总成本 = 佣金 + 印花税 + 过户费 + 其他费用
    
    该结构主要用于存放成本记录结果，一般当做struct直接使用，
    该类本身不对总成本进行计算，也不保证上面的公式成立)")

      .def(py::init<>())
      .def(py::init<price_t, price_t, price_t, price_t, price_t>(), py::arg("commission"),
           py::arg("stamptax"), py::arg("transferfee"), py::arg("others"), py::arg("total"))

      .def("__str__", &CostRecord::toString)
      .def("__repr__", &CostRecord::toString)

      .def_readwrite("commission", &CostRecord::commission, "佣金")
      .def_readwrite("stamptax", &CostRecord::stamptax, "印花税")
      .def_readwrite("transferfee", &CostRecord::transferfee, "过户费")
      .def_readwrite("others", &CostRecord::others, "其他费用")
      .def_readwrite("total", &CostRecord::total, "总成本")
      .def(py::self == py::self)

        DEF_PICKLE(CostRecord);
}