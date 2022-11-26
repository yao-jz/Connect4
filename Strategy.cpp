#include <unistd.h>
#include "Point.h"
#include "Strategy.h"
#include "mcts.h"
#include <ctime>

using namespace std;
#define C 0.717
#define SECOND 2.7

inline void next_state(State* s, int x, int y, int type){
    if(x < 0 || x >= MM || y < 0 || y >= NN) return;
    s->board[x * NN + y] = type + 1;
    s->lastY = y;
    s->lastX = x;
    s->last_type = type;
    if(x >= 2){
        return;
    } else if(x == 1){
        if(s->board[y] == 0){
            if((nnoX != 0) || (nnoY != y)){
                return;
            } else{
                s->expand.erase(y);
            }
        }
    } else {
        s->expand.erase(y);
    }
}

inline void set_next_state(State* s, int x, int y, int type){
    if(x < 0 || x >= MM || y < 0 || y >= NN) return;
    s->board[x * NN + y] = type + 1;
    s->lastY = y;
    s->lastX = x;
    s->last_type = type;
}

//必下
Point find_point(State s, bool& found, int type = 1){
    //不合法时输出的点为-1
    State self_win_state(s);
    State other_win_state(s);
    const int lastx = s.lastX;
    const int lasty = s.lastY;
    const int last_type = s.last_type;
    Point self = Point(-1, -1);
    Point other = Point(-1, -1);
    int x;
    for(int i = 0; i < NN; i++){
        x = -1;
        for(int j = MM - 1; j >= 0; j--){
            if(s.board[j * NN + i] == 0 && (nnoX != j || nnoY != i)){
                x = j;
                break;
            }
        }
        if(x == -1) continue;
        set_next_state(&self_win_state, x, i, type);
        set_next_state(&other_win_state, x, i, 1 - type);
        if(self_win_state.is_terminal()){
            if(self_win_state.win_type == type){
                self = Point(x, i);
                found = true;
                return self;
            }
        }
        self_win_state.board[x * NN + i] = 0;
        self_win_state.lastX = lastx;
        self_win_state.lastY = lasty;
        self_win_state.last_type = last_type;
        if (other_win_state.is_terminal()) {
            if(other_win_state.win_type == 1 - type){
                other = Point(x, i);
                found = true;
            }
        }
        other_win_state.board[x * NN + i] = 0;
        other_win_state.lastX = lastx;
        other_win_state.lastY = lasty;
        other_win_state.last_type = last_type;
    }
    if(found) return other;
    found = false;
    return self;
}

//必不下
set<Point> find_no_point(State s, bool& found, int type = 1){
    set<Point> no_point;
    State s1(s);
    int lastx = s.lastX;
    int lasty = s.lastY;
    int last_type = s.last_type;
    int x = -1;
    for(int i = 0; i < NN; i++){
        x = 0;
        for(int j = MM - 1; j >= 0; j--){
            if(s.board[j * NN + i] == 0 && (nnoX != j ||nnoY != i)){
                x = j;
                break;
            }
        }
        if(x == -1 || x == 0) continue;
        set_next_state(&s1, x - 1, i, 1 - type);
        if(s1.is_terminal()){
            if(s1.win_type == 1 - type){
                no_point.insert(Point(x, i));
            }
        } 
        s1.board[(x - 1) * NN + i] = 0;
        s1.lastX = lastx;
        s1.lastY = lasty;
        s1.last_type = last_type;
    }
    if(no_point.size() != 0) found = true;
    return no_point;
}

set<Point> find_no_point1(State s, bool& found, int type = 1){
    set<Point> no_point;
    State s2(s);
    int lastx = s.lastX;
    int lasty = s.lastY;
    int last_type = s.last_type;
    int x = -1;
    for(int i = 0; i < NN; i++){
        x = 0;
        for(int j = MM - 1; j >= 0; j--){
            if(s.board[j * NN + i] == 0 && (nnoX != j || nnoY != i)){
                x = j;
                break;
            }
        }
        if(x == -1 || x == 0) continue;
        set_next_state(&s2, x, i, 1 - type);
        set_next_state(&s2, x - 1, i, type);
        if(s2.is_terminal()){
            if(s2.win_type == type){
                no_point.insert(Point(x, i));
            }
        } 
        s2.board[(x - 1) * NN + i] = 0;
        s2.board[x * NN + i] = 0;
        s2.lastX = lastx;
        s2.lastY = lasty;
        s2.last_type = last_type;
    }
    if(no_point.size() != 0) found = true;
    return no_point;
}

inline double compute_value(Mcts_Node* n, Mcts_Node* result, double c){
    return double(result->win) / (double(result->visited)) + c * sqrt(2 * log(double(n->visited)) / (double(result->visited)));
}

inline Mcts_Node* best_child(Mcts_Node* n, double c){
    //选择最大值
    Mcts_Node* result;
    double value = 0;
    if(n->first_child) {
        result = n->first_child;
        value = compute_value(n, result, c);
    }
    double temp_value = 0;
    for(auto p = n->first_child->next; p != nullptr; p = p->next){
        temp_value = compute_value(n, p, c);
        if(temp_value > value){
            result = p;
            value = temp_value;
        }
    }
    return result;
}

Mcts_Node* expand(Mcts_Node* v){
    auto it = v->state.expand.begin();
    if(v->state.expand.size() != 1)
        advance(it, rand()%(v->state.expand.size() - 1));
    int move = *it;
    v->state.expand.erase(it);
    return v->new_node(move);
}

Mcts_Node* tree_policy(Mcts_Node* v){
    while(!v->state.is_terminal()){
        if(v->can_expand()){
            return expand(v);
        } else {
            v = best_child(v, C);
        }
    }
    return v;
}

int default_policy(State state){
//    cout << "default ";
    int win = 0;
    int y = -1;
    int x = -1;
    int last_type = state.last_type;
    bool self_found = false;
    Point must = Point(-1, -1);
    Point temp_point = Point(-1, -1);
    auto it = state.expand.begin();
    while(!state.is_terminal()){
        // self_found = false;
        // must = temp_point;
        // must = find_point(state, self_found, 1 - state.last_type);
        // if(self_found){
        //     next_state(&state, must.x, must.y, 1 - state.last_type);
        //     continue;
        // }
        it = state.expand.begin();
        if(state.expand.size() != 1)
            advance(it, rand()%(state.expand.size() - 1));
        y = *it;
        for(int i = MM - 1; i >= 0; i--){
            if(state.board[i * NN + y] == 0 && (nnoX != i || nnoY != y)){
                x = i;
                break;
            }
        }
        next_state(&state, x, y, 1 - state.last_type);
    }
    if(state.win_type == 1 - last_type){
        win = -1;
    } else if (state.win_type == last_type){
        win = 1;
    } else {
        win = 0;
    }
    return win;
}

void backup(Mcts_Node* v, int win) {
    while(v){
        v->win += win;
        win = -1 * win;
        v->visited++;
        v = v->parent;
    }
}
set<Point> no_point;
set<Point> no_point1;
Mcts_Node* root = nullptr;

Point uct_search(State s, bool& must_xia){
    clock_t a,b;
    a = clock();
    State temp_state(s);
    bool found_point = false;
    bool found_no_point = false;
    Point p = find_point(temp_state, found_point);
    no_point = find_no_point(temp_state, found_no_point);
    no_point1 = find_no_point1(temp_state, found_no_point);
    if(found_point) {
        must_xia = true;
        return p;}
    must_xia = false;
    root = get_new_node(s, s.lastX, s.lastY, s.last_type);
    for(auto i = no_point.begin(); i != no_point.end(); i++){
        if(root->state.expand.size() != 1)
            root->state.expand.erase(i->y);
    }
    for(auto i = no_point1.begin(); i != no_point1.end(); i++){
        if(root->state.expand.size() != 1)
            root->state.expand.erase(i->y);
    }
    
    no_point.clear();
    no_point1.clear();
    int win;
    int cnt = 0;
    while(cnt++ < TIMES){
        Mcts_Node* v = tree_policy(root);
        win = default_policy(v->state);
        backup(v, win);
        b = clock();
        if((double(b - a) / CLOCKS_PER_SEC ) > SECOND){
            cerr << cnt << endl;
            break;
       } 
    }
    auto last_root = root;
    // for(auto p = root->first_child; p != nullptr; p = p->next){
    //     cout << "位置为" << " ";
    //     cout << p->state.lastX << " " << p->state.lastY;
    //     cout << "有" << p->visited << "次，赢了" << p->win << "次" << endl;
    // }

    root = best_child(root, C);
    int x = root->state.lastX; int y = root->state.lastY;
    clear_node(last_root);
    return Point(x, y);
}

/*
	策略函数接口,该函数被对抗平台调用,每次传入当前状态,要求输出你的落子点,该落子点必须是一个符合游戏规则的落子点,不然对抗平台会直接认为你的程序有误
	
	input:
		为了防止对对抗平台维护的数据造成更改，所有传入的参数均为const属性
		M, N : 棋盘大小 M - 行数 N - 列数 均从0开始计， 左上角为坐标原点，行用x标记，列用y标记
		top : 当前棋盘每一列列顶的实际位置. e.g. 第i列为空,则_top[i] == M, 第i列已满,则_top[i] == 0
		_board : 棋盘的一维数组表示, 为了方便使用，在该函数刚开始处，我们已经将其转化为了二维数组board
				你只需直接使用board即可，左上角为坐标原点，数组从[0][0]开始计(不是[1][1])
				board[x][y]表示第x行、第y列的点(从0开始计)
				board[x][y] == 0/1/2 分别对应(x,y)处 无落子/有用户的子/有程序的子,不可落子点处的值也为0
		lastX, lastY : 对方上一次落子的位置, 你可能不需要该参数，也可能需要的不仅仅是对方一步的
				落子位置，这时你可以在自己的程序中记录对方连续多步的落子位置，这完全取决于你自己的策略
		noX, noY : 棋盘上的不可落子点(注:涫嫡饫锔?龅膖op已经替你处理了不可落子点，也就是说如果某一步
				所落的子的上面恰是不可落子点，那么UI工程中的代码就已经将该列的top值又进行了一次减一操作，
				所以在你的代码中也可以根本不使用noX和noY这两个参数，完全认为top数组就是当前每列的顶部即可,
				当然如果你想使用lastX,lastY参数，有可能就要同时考虑noX和noY了)
		以上参数实际上包含了当前状态(M N _top _board)以及历史信息(lastX lastY),你要做的就是在这些信息下给出尽可能明智的落子点
	output:
		你的落子点Point
*/
int started = 0;

extern "C" Point* getPoint(const int M, const int N, const int* top, const int* _board, 
	const int lastX, const int lastY, const int noX, const int noY){
	/*
		不要更改这段代码
	*/
    MM = M;
    NN = N;
    nnoX = noX;
    nnoY = noY;
	int x = -1, y = -1;//最终将你的落子点存到x,y中
	int** board = new int*[M];
	for(int i = 0; i < M; i++){
		board[i] = new int[N];
		for(int j = 0; j < N; j++){
			board[i][j] = _board[i * N + j];
		}
	}
    
    int* this_board = new int[M * N];
    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            this_board[i * N + j] = _board[i * N + j];
        }
    }
    // cout << "对方下在了" << lastX << " " << lastY << endl;
    // cout << "上一个位置为" << endl;
    // for(int i = 0 ;i < M; i++){
    //     for(int j = 0; j < N; j++){
    //         cout << this_board[i * N + j] << " " ;
    //     }
    //     cout << endl;
    // }
    if(started == 0){
        head->next_free = &nodes[0];
        for(int i = 0; i < 679998; i++){
            nodes[i].next_free = &nodes[i + 1];
        }
        started = 1;
    }
    
    
    bool must_xia = false;
    State this_state(this_board, lastX, lastY, 0);

    Point p = uct_search(this_state, must_xia);
    x = p.x;
    y = p.y;
    if(!must_xia){
        //判断连续两个的进行拦截
        for(int i = 1; i < NN - 2; i++){
            if((top[i] == top[i + 1]) && (top[i] < M) && (this_board[top[i] * NN + i] == this_board[top[i+1]*NN+i+1]) && this_board[top[i]*NN+i] == 1){
                if(top[i - 1] == top[i] + 1 && top[i + 2] == top[i] + 1){
                    delete[] this_board;
                    clearArray(M, N, board);
                    // cout << "拦截连续的两个" << top[i - 1] - 1 << " " << i - 1 << endl;
                    return new Point(top[i - 1] - 1, i - 1);
                }
                if(top[i - 1] == top[i] + 1 && top[i + 2] != top[i] + 1 && this_board[top[i+1]*NN+i+2] != 2 && (nnoX != top[i+1] || nnoY != i + 2)){
                    delete[] this_board;
                    clearArray(M, N, board);
                    // cout << "拦截连续的两个" << top[i - 1] - 1 << " " << i - 1 << endl;
                    return new Point(top[i - 1] - 1, i - 1);
                }
                if(top[i - 1] != top[i] + 1 && top[i + 2] == top[i] + 1 && this_board[top[i]*NN+i-1] != 2 && (nnoX != top[i] || nnoY != i - 1)){
                    delete[] this_board;
                    clearArray(M, N, board);
                    // cout << "拦截连续的两个" << top[i + 2] - 1 << " " << i + 2 << endl;
                    return new Point(top[i + 2] - 1, i + 2);
                }
            }
        }
    }
    // cerr << "下在了 " << x << "  " << y << endl;
	/*
		不要更改这段代码
	*/
    delete[] this_board;
	clearArray(M, N, board);
	return new Point(x, y);
}


/*
	getPoint函数返回的Point指针是在本dll模块中声明的，为避免产生堆错误，应在外部调用本dll中的
	函数来释放空间，而不应该在外部直接delete
*/
extern "C" void clearPoint(Point* p){
	delete p;
	return;
}

/*
	清除top和board数组
*/
void clearArray(int M, int N, int** board){
	for(int i = 0; i < M; i++){
		delete[] board[i];
	}
	delete[] board;
}


/*
	添加你自己的辅助函数，你可以声明自己的类、函数，添加新的.h .cpp文件来辅助实现你的想法
*/
