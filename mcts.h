#ifndef mcts_hpp
#define mcts_hpp
#include <stdio.h>
#include <cmath>
#include "Point.h"
#include <iostream>
#include "Judge.h"
#include <iterator>
#include <set>
using namespace std;
#define TIMES 680000//模拟的次数
//我是1

int nnoX = 0;
int nnoY = 0;
int MM = 0;
int NN = 0;

struct State {
    //当前棋局的状态
    int board[146];
    int lastX;
    int lastY;
    int last_type = -1;//0表示用户，1表示computer
    int win_type = -1;//0表示用户，1表示computer，2表示平局
    set<int> expand;//可以扩展的set
    State() = default;
    State(int* _board, const int llastX, const int llastY, int last) : lastX(llastX), lastY(llastY), last_type(last){
        for(int i = 0; i < MM; i++){
            for(int j = 0; j < NN; j++){
                this->board[i * NN + j] = _board[i * NN + j];
            }
        }
        if(llastX != -1)
            this->board[llastX * NN + llastY] = last + 1;
        this->expand.clear();
        this->calculate_expand();
    }
    State(const State& s) : lastX(s.lastX), lastY(s.lastY), last_type(s.last_type), win_type(s.win_type) {
        for(int i = 0; i < MM; i++){
            for(int j = 0; j < NN; j++){
                this->board[i * NN + j] = s.board[i * NN + j];
            }
        }
        this->expand.clear();
        this->calculate_expand();
    }
    bool is_terminal(){
        if(lastX == -1) return false;
        if(last_type == 0){//上一次为用户
            if(userWin1(this->lastX, this->lastY, MM, NN, board)){
                this->win_type = 0;
                return true;
            }
        } else if(last_type == 1) {//上一次为computer
            if(machineWin1(this->lastX, this->lastY, MM, NN, board)){
                this->win_type = 1;
                return true;
            }
        }
        //win_type = 2 平局
        bool pingju = true;
        for(int i = 0; i < NN; i++){
            if(this->board[i] == 0){
                if(nnoX == 0 && nnoY == i){
                    if(this->board[i + NN] == 0) {
                        pingju = false;
                        break;
                    }
                } else {
                pingju = false;
                break;}
            }
        }
        if(pingju){
            this->win_type = 2;
            return true;
        }
        return false;
    }
    
    inline void calculate_expand(){
        for(int i = 0; i < NN; i++){
            for(int j = MM - 1; j >= 0; j--){
                if(board[j * NN + i] == 0 && (nnoX != j || nnoY != i)){
                    this->expand.insert(i);
                    break;
                }
            }
        }
    }
    
    inline bool can_expand() {
        return (this->expand.size() != 0) ? true : false;
    }
};
struct Mcts_Node;
inline Mcts_Node* get_new_node(State& s, int x, int y, int last_type);
struct Mcts_Node{
    Mcts_Node* parent = nullptr;
    Mcts_Node* first_child = nullptr;
    Mcts_Node* next = nullptr;//弟弟

    Mcts_Node* next_free = nullptr; 
   
    int visited = 0;//访问的次数
    int win = 0;//胜利的次数
    State state;//这个节点的状态

    
    inline void set_node_state(State& s, int x, int y, int last_type){
        for(int i = 0; i < MM; i++){
            for(int j = 0; j < NN; j++){
                state.board[i * NN + j] = s.board[i * NN + j];
            }
        }
        if(x != -1)
            state.board[x * NN + y] = last_type + 1;
        state.lastX = x;
        state.lastY = y;
        state.win_type = s.win_type;
        state.last_type = last_type;
        state.expand.clear();
        state.calculate_expand();
    }
    //创造子节点
    Mcts_Node* new_node(int y){
        //在第y列增加一个子
        int x = -1;
        for(int i = MM - 1; i >= 0; i--){
            if(this->state.board[i * NN + y] == 0 && (nnoX != i || nnoY != y)){
                x = i;
                break;
            }
        }
        Mcts_Node* new_node = get_new_node(this->state, x, y, (this->state.last_type == 0) ? 1 : 0);
        //指针操作
        new_node->parent = this;
        if(this->first_child){
            Mcts_Node* temp_child = this->first_child;
            while(temp_child){
                if(!temp_child->next){
                    temp_child->next = new_node;
                    break;
                }
                temp_child = temp_child->next;
            }
        } else {
            this->first_child = new_node;
        }
        return new_node;
    }
    
    //可扩展的
    inline bool can_expand() {
        return (this->state.expand.size() > 0) ? true : false;
    }
};
Mcts_Node* nodes = new Mcts_Node[680000];
Mcts_Node* head = new Mcts_Node();
inline Mcts_Node* get_new_node(State& s, int x, int y, int last_type){
    Mcts_Node* result = head->next_free;
    head->next_free = head->next_free->next_free;
    result->set_node_state(s, x, y, last_type);
    result->parent = nullptr;
    result->first_child = nullptr;
    result->next = nullptr;
    result->visited = 0;
    result->win = 0;
    return result;
}
void clear_node(Mcts_Node* n){
    for(auto p = n->first_child; p != nullptr; p = p->next){
        clear_node(p);
    }
    n->next_free = head->next_free;
    head->next_free = n;
    n->visited = 0;
    n->win = 0;
}
#endif /* mcts_hpp */
