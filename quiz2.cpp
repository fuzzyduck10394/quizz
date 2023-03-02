#include <time.h>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

using namespace std;
const int co_ile_revision = 5;
const int co_ile_powtorzenie = 5;
const string nazwa_pliku = "poprawione_dzial3.txt";

// todo
// usprawnione losowanie: % poprawnych odpowiedzi
// jezeli kazda odpowiedz byla zadana co najmniej x razy i ma poprawnosc przynajmniej 80%, wyswietl komunikat "juz umiesz"
// pytanie wspak - uzytkownik mowi pytanie gdy widzi odpowiedz (tlumaczenie z niemieckiego na polski)
// plik z zapisem najtrudniejszych slowek aby przy ponownym wlaczeniu programu je przypomniec
// tryb nauki
// jezeli trzy pierwsze odpowiedzi na dane pytanie byly poprawne, wyrzuc z puli pytan - uzytkownik umial na nie odpowiedziec juz przed wlaczeniem programu, nie ma nic do nauki
// usprawnione revision: niezerowanie calej tablicy
//      a) jezeli uzytkownik odpowiedzial dobrze: zmniejszenie mistaken dwa razy, w przeciwnym wypadku brak zmiany
//      b) jezeli nadchodzi revision i sa dwie odpowiedzi ktore maja ta sama ilosc missów, program wybiera jedna losowo po czym zapomina o drugiej

// global because of the function
vector<string> question;
vector<string> answer;

// global because starting at 0
bool in_lastf[1000];
bool used_que[1000];
int mistaken[1000];
int correct[1000];

void write_vectors(string line) {  // adding char by char to a string, which is finally going to the vector<string>question wether vector<string>answer
    if (line.size() < 2 || line[0] == '/') return;
    string que;
    string ans;
    for (int i = 0; i < line.size(); i++) {
        if (line[i] == '\n' || line[i] == '\t') continue;
        // if line[i] is a start of an answer...
        if (line[i] >= '0' && line[i] <= '9' || line[i] == '-') {
            if (line[i] == '-' || line[i] == '\n') i++;
            while (i < line.size()) {
                ans += line[i];
                i++;
            }
            // if it is not: then it is a question
        } else
            que += line[i];
    }
    question.push_back(que);
    answer.push_back(ans);
}
void mistaken_to_zero() {
    for (int i = 0; i < 1000; i++) {
        mistaken[i] = 0;
    }
}
bool compare_strings(string s1raw, string s2raw) {
    string s1, s2;
    for (int i = 0; i < s1raw.size(); i++) {
        if (s1raw[i] != ' ') s1 += s1raw[i];
    }
    for (int i = 0; i < s2raw.size(); i++) {
        if (s2raw[i] != ' ') s2 += s2raw[i];
    }
    if (s1.size() != s2.size()) return false;

    for (int i = 0; i < s1.size(); i++) {
        if (s1[i] != s2[i]) {
            if ((int)s1[i] + 32 == (int)s2[i]) continue;
            if ((int)s1[i] - 32 == (int)s2[i]) continue;
            return false;
        }
    }
    return true;
}

vector<int> unpack_parts(string raw) {
    vector<int> res;
    for (int i = 0; i < raw.size(); i++) {
        if (raw[i] >= '0' && raw[i] <= '9') res.push_back(int(raw[i]) - 48);
        // make 1-4 equals to 1, 2, 3, 4 and allow an user to make mistakes
        else if (raw[i] == '-' && i != 0 && i + 1 != raw.size() - 1) {
            for (int j = int(raw[i - 1]) - 48; j < int(raw[i + 1]) - 48; j++) {
                res.push_back(j);
            }
        }
    }
    return res;
}

string read_line(string raw_text, int num_of_line) {
    string res = "";
    int line_counter = 0;
    for (int i = 0; i < raw_text.size() && line_counter <= num_of_line; i++) {
        char c = raw_text[i];

        if (c == '\n') line_counter++;
        if (line_counter == num_of_line) res += c;
    }
    return res;
}

int main() {
    // welcome screen
    cout << "QUIZ \nOdpowiadaj na pytania. Nie musisz dbac o wielkosc liter. \nZa kazda dobra odpowiedz otrzymujesz 1 punkt a za zla -1 punkt. Powodzenia!\n";
    cin.get();

    // open a file
    fstream plik;
    plik.open(nazwa_pliku, ios::in);
    if (!plik.good()) {
        cout << "Plik z odpowiedziami nie istnieje.";
        exit(0);
    }

    string line, file_without_comments = "";
    int NUM_OF_QUESTIONS = 0;
    while (getline(plik, line)) {
        if (line.size() < 2 || line[0] == '/') continue;

        NUM_OF_QUESTIONS++;
        file_without_comments += line + '\n';
    }

    string parts;
    if (NUM_OF_QUESTIONS / 10 + 1 != 1) {
        cout << "Quiz zostal podzielony na " << NUM_OF_QUESTIONS / 10 + 1 << " czesci po 10 pytan. Ktore z czesci chcesz dzisiaj przecwiczyc? Jezeli chcesz przecwiczyc wszystkie z nich, napisz 'w'.\n";
        getline(cin, parts);

        // set bool array which says if a question is used or not
        if (parts[0] != 'w') {
            vector<int> unpacked_parts = unpack_parts(parts);
            for (auto i : unpacked_parts) {
                // user's mistake
                if (i > NUM_OF_QUESTIONS / 10 + 1) continue;

                for (int j = (i - 1) * 10; j < (i - 1) * 10 + 10 && j < NUM_OF_QUESTIONS; j++)
                    used_que[j] = true;
            }
        }
    } else
        parts = "w";

    // using the bool array, writing questions to the vector
    for (int i = 0; i < NUM_OF_QUESTIONS; i++) {
        line = read_line(file_without_comments, i);
        if (used_que[i] || parts[0] == 'w') write_vectors(line);
    }

    // ---------------------------------------------------------- START OF THE PROGRAM
    srand(time(NULL));
    int counter = 0;
    int score = 0;
    int randint;
    queue<int> lastf;  // last five

    // hardest is for the revision and easiest is not asked
    // ind, var
    pair<int, int> hardest = {0, -1};
    pair<int, int> easiest = {-1, -1};
    while (1) {
        system("cls");
        cout << "SCORE: " << score << '\n';
        if (counter >= co_ile_revision && hardest.second <= 0) counter = 0;

        // random question which wasnt asked in X last rounds
        if (counter < co_ile_revision) {
            do {
                randint = rand() % question.size();
            } while (in_lastf[randint] || easiest.first == randint);

            lastf.push(randint);
            in_lastf[randint] = true;

            if (lastf.size() > co_ile_powtorzenie) {
                in_lastf[lastf.front()] = false;
                lastf.pop();
            }
        }
        // REVISION TIME
        else {
            // if the hardest question was asked a turn ago
            if (randint == hardest.first) {
                counter = 0;
                continue;
            }
            system("cls");
            cout << "REVISION\n";
            randint = hardest.first;

            // reset
            counter = 0;
            hardest.second = -1;
            mistaken_to_zero();
        }
        cout << question[randint] << "\n\n";

        // check the answer
        string ans;
        getline(cin, ans);
        if (compare_strings(ans, answer[randint])) {
            score++;
            correct[randint]++;
            if (correct[randint] > easiest.second) {
                easiest.first = randint;
                easiest.second = correct[randint];
            }
        } else {
            score--;
            mistaken[randint]++;

            // do some updates for rewind
            if (mistaken[randint] > hardest.second) {
                hardest.second = mistaken[randint];
                hardest.first = randint;
            }
            cout << answer[randint] << '\n';
            cin.get();
        }
        counter++;
    }
}
