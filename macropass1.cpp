#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
using namespace std;

struct MNTEntry {
    string name;
    int mdtIndex;
    int pp, kp;     // positional & keyword parameter counts
    int kpdtIndex;  // start index in KPDTAB
};

struct KPDTEntry {
    string param;
    string value;
};

string trim(string s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.erase(s.begin());
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.pop_back();
    return s;
}

int main() {
    ifstream fin("input.asm");
    ofstream mntf("mnt.txt"), mdtf("mdt.txt"), kpdtf("kpdt.txt"), pntf("pntab.txt"), inter("intermediate.txt");

    if (!fin) {
        cout << "Error: Cannot open input.asm\n";
        return 1;
    }

    vector<MNTEntry> MNT;
    vector<string> MDT;
    vector<KPDTEntry> KPDTAB;
    vector<string> PNTAB;

    string line;
    int mdtPtr = 1, kpdtPtr = 1;

    while (getline(fin, line)) {
        line = trim(line);
        if (line.empty()) continue;

        if (line == "MACRO") {
            // --- Macro Header ---
            getline(fin, line);
            line = trim(line);
            stringstream ss(line);
            string macroName;
            ss >> macroName;

            MNTEntry m;
            m.name = macroName;
            m.mdtIndex = mdtPtr;
            m.pp = 0;
            m.kp = 0;
            m.kpdtIndex = kpdtPtr;

            string param;
            map<string, int> paramIndex;
            while (getline(ss, param, ',')) {
                param = trim(param);
                if (param.empty()) continue;

                if (param[0] == '&') param = param.substr(1);

                size_t eq = param.find('=');
                if (eq != string::npos) {
                    string pName = param.substr(0, eq);
                    string defVal = param.substr(eq + 1);
                    m.kp++;
                    KPDTAB.push_back({pName, defVal});
                    PNTAB.push_back(pName);
                    paramIndex[pName] = PNTAB.size();
                    kpdtPtr++;
                } else {
                    m.pp++;
                    PNTAB.push_back(param);
                    paramIndex[param] = PNTAB.size();
                }
            }

            MNT.push_back(m);

            // --- Macro Body ---
            while (getline(fin, line)) {
                line = trim(line);
                if (line == "MEND") {
                    MDT.push_back("MEND");
                    mdtPtr++;
                    break;
                }

                // Replace &PARAM with (P,index)
                for (auto &p : paramIndex) {
                    string target = "&" + p.first;
                    size_t pos = line.find(target);
                    if (pos != string::npos)
                        line.replace(pos, target.size(), "(P," + to_string(p.second) + ")");
                }

                MDT.push_back(line);
                mdtPtr++;
            }
        } else {
            inter << line << "\n";
        }
    }

    // --- Write MNT ---
    for (auto &m : MNT)
        mntf << m.name << " " << m.mdtIndex << " " << m.pp << " " << m.kp << " " << m.kpdtIndex << "\n";

    // --- Write MDT ---
    for (auto &l : MDT)
        mdtf << l << "\n";

    // --- Write KPDTAB ---
    for (auto &k : KPDTAB)
        kpdtf << k.param << " " << k.value << "\n";

    // --- Write PNTAB ---
    for (auto &p : PNTAB)
        pntf << p << "\n";

    cout << " Pass-I Macro Processing Completed.\n";
    cout << "Files generated: mnt.txt, mdt.txt, kpdt.txt, pntab.txt, intermediate.txt\n";

    return 0;
}
