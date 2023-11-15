#pragma once

#include <fstream>
#include <tuple>
#include <utility> 
#include <string>
#include <vector>

namespace cle {

template<typename... As> 
struct HetList {
    HetList(As... as) : HetList<As...>(as...) {}
};

template<typename A, typename... As> 
struct HetList<A, As...> {
    A head;
    HetList<As...> tail;
    bool last = false;
    HetList(A head, As... tail) : head(head), tail(tail...) {}
};

template<typename A> 
struct HetList<A> { 
    A head;
    bool last = true;
    HetList(A head) : head(head) {} 
};

template<typename... As>
struct TableAux {
    static void output_row(HetList<As...> row, std::ofstream& stream, std::string delim);
};

template<typename A, typename... As>
struct TableAux<A, As...> {
    static void output_row(HetList<A, As...> row, std::ofstream& stream, std::string delim) {
        stream << row.head << delim;
        TableAux<As...>::output_row(row.tail, stream, delim);
    }
};

template<typename A>
struct TableAux<A> {
    static void output_row(HetList<A> row, std::ofstream& stream, std::string delim) {
        stream << row.head;
    }
};


template<typename... As>
class Table {
public:
    using Row = HetList<As...>;
    using Rows = std::vector<Row>;

    Table(Rows rows) : rows(rows) {}
    Table() : rows(std::vector<Row>()) {}

    Table operator <<(Row r) {
        rows.push_back(r);
        return *this;
    }
    Row operator [](int index) { return rows[index]; }

    void output_csv(std::ofstream& stream, std::string delim, std::string newline) {
        for(auto row : rows) {
            TableAux<As...>::output_row(row, stream, delim);
            stream << newline;
        }
    }

private:
    Rows rows; 

};



};