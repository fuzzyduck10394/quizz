#include <bits/stdc++.h>
using namespace std;
const int co_ile_revision = 10;
const int co_ile_powtorzenie = 10;
const string nazwa_pliku = "poprawione_dzial3.txt";

// global because of the function
vector<string> question;
vector<string> answer;

// global because starting at 0
bool in_lastf[1000];
int mistaken[1000];

void write_vectors(string line) {
    if (line.size() < 2 || line[0] == '/') return;
    string que;
    string ans;
    for (int i = 0; i < line.size(); i++) {
        if ((int)line[i] >= 48 && (int)line[i] <= 57 || line[i] == '-') {
            if (line[i] == '-') i++;
            while (i < line.size()) {
                ans += line[i];
                i++;
            }
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
    string line;
    while (getline(plik, line)) {
        write_vectors(line);
    }
    int counter = 0;
    int score = 0;
    int randint;
    queue<int> lastf;  // last five
    // ind, var
    pair<int, int> hardest = {0, -1};
    while (1) {
        system("clear");
        cout << "SCORE: " << score << '\n';
        if (counter >= co_ile_revision && hardest.second <= 0) counter = 0;

        // random question which wasnt asked in X last rounds
        if (counter < co_ile_revision) {
            do {
                randint = rand() % question.size();
            } while (in_lastf[randint]);

            lastf.push(randint);
            in_lastf[randint] = true;

            if (lastf.size() > co_ile_powtorzenie) {
                in_lastf[lastf.front()] = false;
                lastf.pop();
            }
        }
        // REVISION TIME
        else {
            if (randint == hardest.first) {
                counter = 0;
                continue;
            }
            system("clear");
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
        } else {
            score--;
            mistaken[randint]++;

            // do some updates for rewind
            if (mistaken[randint] > hardest.second) {
                hardest.second = mistaken[randint];
                hardest.first = randint;
            }
            cout << answer[randint];
            cin.get();
        }
        counter++;
    }
}