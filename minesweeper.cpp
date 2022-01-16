#include <iostream>
#include <random>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <fstream>
#include <string>

using namespace std;

const int maxWait = 100; 
const int N = 25, M = 500;

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */

int n, bombs, a[N][N], b[N][N], flag_count, cnt_good, pos_x, pos_y;
double time_limit;
bool used[N][N], flagged[N][N];
string player_name;
int dx[] = {1, 1, 1, -1, -1, -1, 0, 0};
int dy[] = {1, -1, 0, 1, -1, 0, 1, -1};

void main_menu();
void input();

struct Person {
    string name;
    int points;
    Person() {
        points = 0;
    }
}player;

bool operator< (Person A, Person B) {
    if(A.points != B.points)
        return A.points > B.points;
    return A.name < B.name;
}

int StringtoInt(string st) {
    int res = 0;
    for (auto x : st)
        res *= 10, res += (x - '0');
    return res;
}

void create_game() {
    pos_x = pos_y = 0;
    input();
    flag_count = cnt_good = 0;
    memset(a, 0, sizeof a);
    memset(b, 0, sizeof b);
    memset(used, 0, sizeof used);
    memset(flagged, 0, sizeof flagged);
    srand(time(NULL));
    for (int i = 0; i < bombs; i++) {
        int num = rand() % (n * n);
        int x = num / n, y = num % n;
        if(a[x][y] == 1) {
            i--;
            continue;
        }
        a[x][y] = 1;
    }
}

void clean() {
    system("clear");
}

char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}

void print() {
    clean();
    for (int i = 0; i < n; i++, cout << endl) {
        for (int j = 0; j < n; j++) {
            if(pos_x == i && pos_y == j) {
                cout << "! ";
                continue;
            }
            if(flagged[i][j])
                cout << "F ";
            else {
                if(!used[i][j])
                    cout << "- ";
                else {
                    if(b[i][j] == 0)
                        cout << "  ";
                    else
                        cout << b[i][j] << " ";
                }
            }
        }
    }
    cout << endl << endl;
}

void print_L() {
    clean();
    for (int i = 0; i < n; i++, cout << endl) {
        for (int j = 0; j < n; j++) {
            if(a[i][j]) {
                cout << "* ";
                continue;
            }
            if(flagged[i][j])
                cout << "F ";
            else {
                if(!used[i][j])
                    cout << "- ";
                else {
                    if(b[i][j] == 0)
                        cout << "  ";
                    else
                        cout << b[i][j] << " ";
                }
            }
        }
    }
}

bool valid(int x, int y) {
    if(min(x, y) < 0 || max(x, y) >= n)
        return false;
    return true;
}

void add(int x, int y) {
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i], ny = y + dy[i];
        if(valid(nx, ny) && a[nx][ny] == 1)
            b[x][y]++;
    }
}

void prep() {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if(!a[i][j])
                add(i, j);
}

void dfs(int x, int y) {
    cnt_good++;
    used[x][y] = true;
    if(b[x][y])
        return;
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i], ny = y + dy[i];
        if(valid(nx, ny) && !a[nx][ny] && !flagged[nx][ny] && !used[nx][ny])
            dfs(nx, ny);
    }
}

void play(bool);

void change_player_points(Person &dude);

struct game_file {
    int n, bombs, pos_x, pos_y, flag_count, cnt_good;
    double time_limit;
    int a[N][N], b[N][N];
    bool used[N][N], flagged[N][N];
};

game_file convert_to_gamefile() {
    game_file ret;
    ret.n = n, ret.bombs = bombs, ret.pos_x = pos_x, ret.pos_y = pos_y, ret.flag_count = flag_count, ret.cnt_good = cnt_good, ret.time_limit = time_limit;
    memcpy(ret.a, a, sizeof a), memcpy(ret.b, b, sizeof b), memcpy(ret.used, used, sizeof used), memcpy(ret.flagged, flagged, sizeof flagged);
    return ret;
}

void save_game(game_file now) {
    ifstream MyReadFile("savefile.txt");
    int tc;
    MyReadFile >> tc;
    vector <game_file> vec;
    vec.push_back(now);
    for (int i = 0; i < tc; i++) {
        game_file cur;
        MyReadFile >> cur.n >> cur.bombs >> cur.pos_x >> cur.pos_y >> cur.flag_count >> cur.cnt_good >> cur.time_limit;
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.a[j][k];
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.b[j][k];
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.used[j][k];
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.flagged[j][k];
        vec.push_back(cur);
    }
    MyReadFile.close();
    system(" > savefile.txt");
    ofstream MyFile("savefile.txt");
    MyFile << vec.size() << "\n";
    tc = vec.size();
    for (int i = 0; i < tc; i++) {
        game_file cur = vec[i];
        MyFile << cur.n << " " << cur.bombs << " " << cur.pos_x << " " << cur.pos_y << " " << cur.flag_count << " " << cur.cnt_good << " " << cur.time_limit << "\n";
        for (int j = 0; j < N; j++, MyFile << "\n")
            for (int k = 0; k < N; k++)
                MyFile << cur.a[j][k] << " ";
        for (int j = 0; j < N; j++, MyFile << "\n")
            for (int k = 0; k < N; k++)
                MyFile << cur.b[j][k] << " ";
        for (int j = 0; j < N; j++, MyFile << "\n")
            for (int k = 0; k < N; k++)
                MyFile << cur.used[j][k] << " ";
        for (int j = 0; j < N; j++, MyFile << "\n")
            for (int k = 0; k < N; k++)
                MyFile << cur.flagged[j][k] << " ";
    }
    MyFile.close();
}

void load_file() {
    clean();
    ifstream MyReadFile("savefile.txt");
    int tc, idx = -1;
    MyReadFile >> tc;
    if(!tc) {
        cout << RED << "You have don't have any saved files\n" << RESET;
        return main_menu();
    }
    vector <game_file> vec;
    for (int i = 0; i < tc; i++) {
        game_file cur;
        MyReadFile >> cur.n >> cur.bombs >> cur.pos_x >> cur.pos_y >> cur.flag_count >> cur.cnt_good >> cur.time_limit;
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.a[j][k];
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.b[j][k];
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.used[j][k];
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                MyReadFile >> cur.flagged[j][k];
        vec.push_back(cur);
    }
    while(true) {
        cout << YELLOW << "Please enter the save file you want to play" << RESET << "\n";
        cout << GREEN;
        for (int i = 0; i < tc; i++)
            cout << i + 1 << ". Size: " << vec[i].n << " | Number of Bombs: " << vec[i].bombs << " | Number of flags: " << vec[i].flag_count << "\n";
        cout << RESET;
        cin >> idx;
        idx--;
        if(idx >= 0 && idx < tc)
            break;
        clean();
        cout << RED << "Invalid Input." << RESET << "\n";
    }
    game_file cur = vec[idx];
    n = cur.n, bombs = cur.bombs, pos_x = cur.pos_x, pos_y = cur.pos_y, flag_count = cur.flag_count, cnt_good = cur.cnt_good, time_limit = cur.time_limit;
    memcpy(a, cur.a, sizeof a), memcpy(b, cur.b, sizeof b), memcpy(used, cur.used, sizeof used), memcpy(flagged, cur.flagged, sizeof flagged);
    MyReadFile.close();
}

void play(bool new_game = 1) {
    if(new_game)
        prep(), time_limit = 0;
    double t = clock();
    print();
    while((time_limit + ((clock() - t) / CLOCKS_PER_SEC) < 100)) {
        time_limit += ((clock() - t) / CLOCKS_PER_SEC);
        char type = getch();
        if(type == 'W' || type == 'w')
            pos_x = (pos_x - 1 + n) % n;
        else if(type == 'A' || type == 'a')
            pos_y = (pos_y - 1 + n) % n;
        else if(type == 's' || type == 'S')
            pos_x = (pos_x + 1) % n;
        else if(type == 'd' || type == 'D')
            pos_y = (pos_y + 1) % n;
        else if(type == ' ') {
            if(flagged[pos_x][pos_y]) {
                cout << "\033[1;31mYou Have Already Called This Coordinate. Please Pick Another Coordinate." << RESET << endl;
                print();
                continue;
            }
            else if(used[pos_x][pos_y]) {
                cout << "\033[1;31mThis Coordinate Is Already Visable. Please Pick Another Coordinate." << RESET << endl;
                print();
                continue;
            }
            if(a[pos_x][pos_y] == 1) {
                cout << "\033[1;31mYOU LOST :(" << RESET << endl;
                print_L();
                player.points -= 10;
                change_player_points(player);
                return main_menu();
            }
            dfs(pos_x, pos_y);
        }
        else if(type == 'F' || type == 'f') {
            if(used[pos_x][pos_y]) {
                cout << "\033[1;31mYou Have Already Called This Coordinate. Please Pick Another Coordinate." << RESET << endl;
                print();
                continue;
            }
            if(flagged[pos_x][pos_y] == 1)
                flag_count--;
            else {
                if(flag_count == bombs) {
                    cout << "\033[1;31mCannot Add Anymore Flags" << RESET << endl;
                    print();
                    continue;
                }
                flag_count++;
            }
            flagged[pos_x][pos_y] ^= 1;
            if(flagged[pos_x][pos_y])
                player.points += ((a[pos_x][pos_y])? 5 : -5);
            else
                player.points -= ((a[pos_x][pos_y])? 5 : -5);
        }
        else if(type == 'Q' || type == 'q') {
            cout << "\033[;34mGame Ended." << RESET << endl;
            print_L();
            change_player_points(player);
            return main_menu();
        }
        else if(type == 'O' || type == 'o') {
            change_player_points(player);
            save_game(convert_to_gamefile());
            clean();
            cout << GREEN << "Game File Saved\n" << RESET << endl;
            return main_menu();
        }
        else {
            cout << "\033[1;31mInvalid Input." << RESET << endl;
            print();
            continue;
        }
        if(cnt_good == n * n - bombs) {
            cout << "\033[;32mNo Way! :O\tYou Won :)" << RESET << endl;
            player.points += min(200, n * bombs);
            change_player_points(player);
            return main_menu();
        }
        print();
    }
    clean();
    cout << RED << "You have reached the timelimit" << RESET << endl;
    player.points -= 10;
    print_L();
    change_player_points(player);
    return main_menu();
}

void input() {
    while(true) {
        cout << YELLOW << "Please select your preferred difficulty" << RESET << endl;
        cout << "1. 5x5 grid with 4 bombs" << endl;
        cout << GREEN << "2. 12x12 grid with 28 bombs" << endl;
        cout << RED << "3. 20x20 grid with 96 bombs" << endl;
        cout << YELLOW << "4. Custom difficulty" << RESET << endl;
        char input_dif = getch();
        if(input_dif >= '1' && input_dif <= '4') {
            if(input_dif == '4') {
                clean();
                break;
            }
            if(input_dif == '1')
                n = 5, bombs = 4;
            else if(input_dif == '2')
                n = 12, bombs = 28;
            else
                n = 20, bombs = 96;
            clean();
            return;
        }
        else {
            clean();
            cout << RED << "Invalid Input" << RESET << endl;
            continue;
        }
    }
    while(true) {
        cout << GREEN << "Please Enter The Size Of The Grid" << RESET << endl;
        cin >> n;
        if(n < 5) {
            clean();
            cout << RED << "Invalid Input" << RESET << endl;
            continue;
        }
        if(n > 20) {
            clean();
            cout << RED << "Please Choose a Smaller Number" << RESET << endl;
            continue;
        }
        break;
    }
    while(true) {
        cout << GREEN << "Please Enter The Number Of Bombs" << RESET << endl;
        cin >> bombs;
        if(bombs < 0 || bombs > n * n) {
            clean();
            cout << RED << "Invalid Input" << RESET << endl;
            continue;
        }
        break;
    }
}

void print_leaderboard() {
    ifstream MyReadFile("leaderboard.txt");
    string t;
    int pts;
    cout << GREEN;
    int tc;
    MyReadFile >> tc;
    for (int i = 0; i < min(5, tc); i++) {
        Person now;
        MyReadFile >> now.name >> now.points;
        cout << "#" << i + 1 << "\t" << now.name << " " << now.points << "\n";
    }
    MyReadFile.close();
    cout << "PRESS ANY BUTTON TO CONTINUE\n";
    cout << RESET;
    char ch = getch();
    clean();
    main_menu();
}

string substring(string &st, int l, int r) {
    string ret;
    for (int i = l; i < r; i++)
        ret.push_back(st[i]);
    return ret;
}

Person get_person(string st) {
    string first_part, second_part;
    int sz = st.size();
    for (int i = 0; i < sz; i++)
        if(st[i] == ' ')
            first_part = substring(st, 0, i), second_part = substring(st, i + 1, sz);
    Person ret;
    ret.points = StringtoInt(second_part);
    ret.name = first_part;
    return ret;
}

void add_player(Person &dude) {
    vector <Person> vec;
    vec.push_back(dude);
    ifstream MyReadFile("leaderboard.txt");
    string t;
//    getline(MyReadFile, t);
//  int tc = StringtoInt(t);
    int tc;
    MyReadFile >> tc;
    for (int i = 0; i < tc; i++) {
        Person now;
        MyReadFile >> now.name >> now.points;
        vec.push_back(now);
    }
    MyReadFile.close();
    sort(vec.begin(), vec.end());
    system("> leaderboard.txt");
    ofstream MyFile("leaderboard.txt");
    MyFile << (int)vec.size() << "\n";
    for (int i = 0; i < vec.size(); i++)
        MyFile << vec[i].name << " " << vec[i].points << "\n";
    MyFile.close();
}

void change_player_points(Person &dude) {
    vector <Person> vec;
    ifstream MyReadFile("leaderboard.txt");
    string t;
    getline(MyReadFile, t);
    int tc = StringtoInt(t);
    for (int i = 0; i < tc; i++) {
        getline(MyReadFile, t);
        Person now = get_person(t);
        if(now.name == dude.name)
            now.points = dude.points;
        vec.push_back(now);
    }
    MyReadFile.close();
    sort(vec.begin(), vec.end());
    system(" > leaderboard.txt");
    ofstream MyFile("leaderboard.txt");
    MyFile << (int)vec.size() << "\n";
    for (int i = 0; i < vec.size(); i++)
        MyFile << vec[i].name << " " << vec[i].points << "\n";
    MyFile.close();
}

void check_player() {
    ifstream MyReadFile("leaderboard.txt");
    string t;
//    getline(MyReadFile, t);
//      int tc = StringtoInt(t);
    int tc;
    MyReadFile >> tc;
    bool found = false;
    for (int i = 0; i < tc; i++) {
        Person now;
        MyReadFile >> now.name >> now.points;
        if(player_name == now.name)
            found = true, player = now;
    }
    MyReadFile.close();
    if(!found) {
        Person dude;
        dude.name = player_name;
        add_player(dude);
        player = dude;
    }
}

void change_name() {
    cout << YELLOW << "Please enter your new name" << RESET << endl;
    cin >> player_name;
    check_player();
    clean();
    main_menu();
}

void main_menu() {
    cout << YELLOW;
    cout << "Welcome to minesweeper " << RED << player_name << endl << RESET;
    cout << "1. New Game" << endl;
    cout << "2. Load Game" << endl;
    cout << "3. Change Name" << endl;
    cout << "4. Leaderboard" << endl;
    cout << "5. Quit" << endl;
    cout << RESET;
    char ch = getch();
    if(ch >= '1' && ch <= '5') {
        clean();
        if(ch == '1') {
            create_game();
            play();
        }
        else if(ch == '2') {
            load_file();
            play(0);
        }
        else if(ch == '3')
            change_name();
        else if(ch == '4')
            print_leaderboard();
        else {
            cout << MAGENTA << "Bye :(" << RESET << endl;
            exit(0);
        }
    }
    else {
        clean();
        cout << RED << "Invalid Input" << RESET << endl;
        return main_menu();
    }
}

void initiate_game() {
    cout << GREEN << "Please Enter Your Name" << endl << RESET;
    cin >> player_name;
    check_player();
    clean();
    main_menu();
}

int main() {
    initiate_game();
}