// mbg_feedback.cpp
// Efisiensi Pengelolaan Feedback MBG
// Kompilasi: g++ -std=c++17 -O2 -o mbg_feedback mbg_feedback.cpp

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <cctype>
#include <set>

using namespace std;

// --------------------------
// Utility: trim helpers
// --------------------------
static inline string &ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) { return !isspace(ch); }));
    return s;
}
static inline string &rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !isspace(ch); }).base(), s.end());
    return s;
}
static inline string &trim(string &s) { return ltrim(rtrim(s)); }

// --------------------------
// Indonesian stopwords (basic list)
// You can expand this list in README or via external file.
// --------------------------
const set<string> STOPWORDS = {
    "yang","dan","di","ke","dari","ini","itu","ada","untuk","pada","dengan",
    "sebagai","atau","sebagai","karena","oleh","sehingga","saat","juga",
    "tidak","bahwa","saya","kami","kita","anda","dia","mereka","adalah",
    "sebuah","seorang","beberapa","banyak","lebih","kurang","tetapi","sudah",
    "belum","akan","masih","dicatat","oleh","dalam","setiap","juga","lagi",
    "lagi","pada","pula","lain","lainnya","pada","oleh","ke","kepada"
};

// --------------------------
// Cleaning: remove non-letters (keeps letters and spaces).
// We'll treat '-' specially for reduplication like "buku-buku".
// --------------------------
string clean_text(const string &s) {
    string out;
    for (char ch : s) {
        unsigned char uch = static_cast<unsigned char>(ch);
        // allow letters, space, hyphen (for redup), and basic Indonesian letters (ASCII letters only here)
        if (isalpha(uch) || isspace(uch) || ch == '-' ) {
            out.push_back(ch);
        } else {
            // replace other characters with space to avoid word-joining
            out.push_back(' ');
        }
    }
    return out;
}

// --------------------------
// Case folding: tolower for ASCII
// --------------------------
string case_fold(const string &s) {
    string r = s;
    for (size_t i = 0; i < r.size(); ++i) r[i] = static_cast<char>(tolower((unsigned char)r[i]));
    return r;
}

// --------------------------
// Tokenizing: split on whitespace, also handle reduplications like "makanan-makanan" or "kata-kata"
// Returns tokens already cleaned/casefolded.
// --------------------------
vector<string> tokenize(const string &s) {
    vector<string> tokens;
    stringstream ss(s);
    string w;
    while (ss >> w) {
        // if token contains '-' and looks like redup "kata-kata", split and take first token
        size_t pos = w.find('-');
        if (pos != string::npos) {
            // split parts and consider each part as a token; often redup has same part twice
            string first = w.substr(0, pos);
            string second = w.substr(pos + 1);
            if (!first.empty()) tokens.push_back(first);
            if (!second.empty() && second != first) tokens.push_back(second);
        } else {
            tokens.push_back(w);
        }
    }
    return tokens;
}

// --------------------------
// Stopword removal
// --------------------------
vector<string> remove_stopwords(const vector<string> &tokens) {
    vector<string> out;
    for (const string &t : tokens) {
        if (t.empty()) continue;
        if (STOPWORDS.find(t) == STOPWORDS.end()) out.push_back(t);
    }
    return out;
}

// --------------------------
// Simple Indonesian stemming (light stemming):
// - remove common suffixes: -lah, -kah, -ku, -mu, -nya, -i, -kan, -an
// - handle redup if still present (we attempted earlier)
// NOTE: This is a heuristic stemmer, not a full algorithm (e.g., Nazief-Adriani).
// --------------------------
string simple_stem(const string &w) {
    string s = w;
    // a list of suffixes ordered by longest first
    const vector<string> suffixes = {"kan","lah","kah","nya","ku","mu","an","i"};
    bool changed = true;
    // try removing suffixes once (do not over-strip)
    for (const string &suf : suffixes) {
        if (s.size() > suf.size() + 2) { // ensure leftover length >=3 to avoid tiny stems
            if (s.size() >= suf.size() && s.substr(s.size() - suf.size()) == suf) {
                s = s.substr(0, s.size() - suf.size());
                break;
            }
        }
    }
    // light normalization: if word ends with double letter due to bad tokenization, trim
    while (s.size() > 2 && s.back() == s[s.size()-2]) s.pop_back();
    return s;
}

// --------------------------
// Frequency counting and top-N extraction
// --------------------------
vector<pair<string,int>> top_n(const map<string,int> &freq, int n) {
    vector<pair<string,int>> all;
    all.reserve(freq.size());
    for (auto &p : freq) all.push_back(p);
    sort(all.begin(), all.end(), [](const pair<string,int>&a, const pair<string,int>&b){
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    if (n <= 0 || n >= (int)all.size()) return all;
    all.resize(n);
    return all;
}

// --------------------------
// Main interactive flow
// --------------------------
int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "=== Sistem Analisis Feedback MBG (versi sederhana) ===\n";
    int n;
    cout << "Masukkan jumlah feedback (ketik 0 untuk membaca dari stdin sampai EOF): ";
    if (!(cin >> n)) {
        cerr << "Input tidak valid\n";
        return 1;
    }
    cin.ignore();

    vector<string> feedbacks;
    if (n > 0) {
        string line;
        for (int i = 0; i < n; ++i) {
            cout << "Feedback " << i+1 << ": ";
            getline(cin, line);
            trim(line);
            feedbacks.push_back(line);
        }
    } else {
        cout << "Masukkan feedback per baris (CTRL+D / CTRL+Z untuk selesai):\n";
        string line;
        while (getline(cin, line)) {
            trim(line);
            if (!line.empty()) feedbacks.push_back(line);
        }
    }

    // parameter: top N
    int topN = 10;
    cout << "Berapa top kata kunci yang ingin ditampilkan? (default 10): ";
    string sn; getline(cin, sn);
    if (!sn.empty()) {
        try { topN = stoi(sn); } catch(...) { topN = 10; }
    }

    // Processing pipeline
    map<string,int> freq;

    for (const string &fb : feedbacks) {
        string s = case_fold(fb);
        s = clean_text(s);
        vector<string> tokens = tokenize(s);
        vector<string> tokens_no_stop = remove_stopwords(tokens);

        for (string t : tokens_no_stop) {
            if (t.empty()) continue;
            string stem = simple_stem(t);
            if (stem.empty()) stem = t;
            freq[stem]++; // count stemmed token
        }
    }

    vector<pair<string,int>> top = top_n(freq, topN);

    // Output template
    cout << "\n=== Output Feedback (Top " << topN << ") ===\n";
    for (auto &p : top) {
        cout << "- " << p.first << " (" << p.second << " kali)\n";
    }

    // Also print full ranked list summary
    cout << "\n=== Daftar lengkap (urut frekuensi) ===\n";
    vector<pair<string,int>> all;
    for (auto &it : freq) all.push_back(it);
    sort(all.begin(), all.end(), [](const pair<string,int>&a, const pair<string,int>&b){
        if (a.second != b.second) return a.second > b.second;
        return a.first < b.first;
    });
    for (auto &p : all) {
        cout << p.first << " : " << p.second << "\n";
    }

    cout << "\nSelesai.\n";
    return 0;
}
