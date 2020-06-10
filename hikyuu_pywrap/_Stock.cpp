/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2020-5-26
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>
#include <hikyuu/KData.h>
#include <hikyuu/Stock.h>
#include <hikyuu/serialization/Stock_serialization.h>
#include "_Parameter.h"
#include "convert_Datetime.h"
#include "pickle_support.h"
#include "StockType.h"
#include "pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_Stock(py::module& m) {
    py::class_<Stock>(m, "Stock", "证券类")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&>(), py::arg("market"),
           py::arg("code"), py::arg("name"))
      .def(py::init<const Stock&>())
      .def("__str__", &Stock::toString)
      .def("__repr__", &Stock::toString)

      .def_property_readonly("market", &Stock::market, py::return_value_policy::copy,
                             "所属市场简称")
      .def_property_readonly("code", &Stock::code, py::return_value_policy::copy, "证券代码")
      .def_property_readonly("market_code", &Stock::market_code, py::return_value_policy::copy,
                             "市场简称+证券代码，如: sh000001")
      .def_property_readonly("name", &Stock::name, py::return_value_policy::copy, "证券名称")
      .def_property_readonly(
        "type", [](const Stock& stock) { return StockType(stock.type()); }, "证券类型")
      .def_property_readonly("valid", &Stock::valid, "该证券当前是否有效")
      .def_property_readonly("start_date", &Stock::startDatetime, "证券起始日期, 上市日期")
      .def_property_readonly("last_date", &Stock::lastDatetime, "证券最后日期，退市日期")
      .def_property_readonly("tick", &Stock::tick, "最小跳动量")
      .def_property_readonly("tick_value", &Stock::tickValue, "最小跳动量价格")
      .def_property_readonly("unit", &Stock::unit, "每单位价值 = tickValue / tick")
      .def_property_readonly("precision", &Stock::precision, "价格精度")
      .def_property_readonly("atom", &Stock::atom, "允许的最小交易数量，同min_trade_number")
      .def_property_readonly("min_trade_number", &Stock::minTradeNumber, "允许的最小交易数量")
      .def_property_readonly("max_trade_number", &Stock::maxTradeNumber, "允许的最大交易数量")

      .def("is_null", &Stock::isNull, "是否为空对象")
      .def("is_buffer", &Stock::isBuffer, "指定类型的K线数据是否被缓存")

      .def("get_kdata", &Stock::getKData, "根据查询条件获取K线数据")

      .def(
        "get_timeline_list",
        [](const Stock& stk, const KQuery& query) {
            return vector_to_python_list(stk.getTimeLineList(query));
        },
        R"(获取分时线
        
:param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）)")

      .def(
        "get_trans_list",
        [](const Stock& stk, const KQuery& q) {
            return vector_to_python_list(stk.getTransList(q));
        },
        R"(获取历史分笔数据

:param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）)")

      .def("get_count", &Stock::getCount, py::arg("ktype") = KQuery::DAY,
           R"(获取指定类型的K线数据量

:param Query.KType ktype: K线数据类别
:return: K线记录数)")

      .def(
        "get_market_value", &Stock::getMarketValue,
        R"(获取指定时刻的市值，即小于等于指定时刻的最后一条记录的收盘价

:param datetime: 指定时刻
:param Query.KType ktype: K线数据类别
:return: 指定时刻的市值)")

      .def("get_krecord", &Stock::getKRecord, py::arg("pos"), py::arg("ktype") = KQuery::DAY,
           R"(获取指定索引的K线数据记录，未作越界检查

:param int pos: 指定的索引位置, 大于0小于相应K线数据总量
:param Query.KType ktype: K线数据类别
)")

      .def("get_krecord_by_date", &Stock::getKRecordByDate, py::arg("date"),
           py::arg("ktype") = KQuery::DAY,
           R"(根据数据类型（日线/周线等），获取指定时刻的KRecord

:param datetime date: 指定日期时刻
:param Query.KType ktype: K线数据类别)")

      .def(
        "get_krecord_list",
        [](const Stock& stk, size_t start, size_t end, KQuery::KType ktype) {
            return vector_to_python_list(stk.getKRecordList(start, end, ktype));
        },
        R"(获取K线记录 [start, end)，一般不直接使用，用get_kdata替代

:param int start: 起始位置
:param int end: 结束位置
:param Query.KType ktype: K线类别
:return: K线记录列表)")

      .def(
        "get_date_list",
        [](const Stock& stk, size_t start, size_t end, KQuery::KType ktype) {
            return vector_to_python_list<Datetime>(stk.getDatetimeList(start, end, ktype));
        },
        R"(获取日期时间列表 [start, end)

:param int start: 起始位置
:param ind end: 结束位置
:param Query.KType ktype: K线类型)")

      .def(
        "get_date_list",
        [](const Stock& stk, const KQuery& q) {
            return vector_to_python_list<Datetime>(stk.getDatetimeList(q));
        },
        R"(获取日期时间列表

:param Query query: 查询条件)")

      // TODO
      .def("get_finance_info", &Stock::getFinanceInfo)
      .def("get_history_finance_info", &Stock::getHistoryFinanceInfo)

      .def("realtime_update", &Stock::realtimeUpdate,
           R"(（临时函数）只用于更新缓存中的日线数据

:param KRecord krecord: 新增的实时K线记录)")

      .def(
        "get_weight", [](const Stock& stk) { return vector_to_python_list(stk.getWeight()); },
        "获取全部权息信息")

      .def(
        "get_weight",
        [](const Stock& stk, const Datetime& start, const Datetime& end) {
            return vector_to_python_list(stk.getWeight(start, end));
        },
        py::arg("start"), py::arg("end") = py::none(),
        R"(获取指定时间段[start,end)内的权息信息。未指定起始、结束时刻时，获取全部权息记录。

:param datetime start: 起始时刻
:param datetime end: 结束时刻, 默认 None)")

      .def("load_kdata_to_buffer", &Stock::loadKDataToBuffer,
           R"(将指定类别的K线数据加载至内存缓存

:param Query.KType ktype: K线类型)")

      .def("release_kdata_buffer", &Stock::releaseKDataBuffer,
           R"(释放指定类别的内存K线数据        

:param Query.KType ktype: K线类型)")

      .def("__hash__", [](const Stock& stock) { return (Py_hash_t)stock.id(); })

      .def("__eq__", &Stock::operator==)
      .def("__ne__", &Stock::operator!=)

        DEF_PICKLE(Stock);

    m.def("get_stock", &getStock, "Return the stock object of the specified code, like 'sh000001'");
}