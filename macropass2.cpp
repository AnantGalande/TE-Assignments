#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <string>
using namespace std;

struct MNTEntry {
    string name;
    int mdtIndex, pp, kp, kpdtIndex;
};

struct KPDTEntry {
    string param, value;
};

int main() {
    ifstream mntf("mnt.txt"), mdtf("mdt.txt"), kpdtf("kpdt.txt"), inter("intermediate.txt");
    ofstream outf("output.txt");

    if (!mntf || !mdtf || !kpdtf || !inter) {
        cout << "Error: One or more input files missing!\n";
        return 1;
    }

    vector<MNTEntry> MNT;
    vector<string> MDT;
    vector<KPDTEntry> KPDTAB;
    vector<string> PNTAB;

    // -------- Load MNT --------
    MNTEntry m;
    while (mntf >> m.name >> m.mdtIndex >> m.pp >> m.kp >> m.kpdtIndex)
        MNT.push_back(m);

    // -------- Load MDT --------
    string line;
    while (getline(mdtf, line)) {
        if (!line.empty())
            MDT.push_back(line);
    }

    // -------- Load KPDTAB --------
    KPDTEntry k;
    while (kpdtf >> k.param >> k.value)
        KPDTAB.push_back(k);

    // -------- Process Intermediate Code --------
    while (getline(inter, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        string word;
        ss >> word;

        bool isMacro = false;
        int macroIndex = -1;

        // Check if line starts with a macro name
        for (int i = 0; i < MNT.size(); i++) {
            if (word == MNT[i].name) {
                isMacro = true;
                macroIndex = i;
                break;
            }
        }

        if (!isMacro) {
            // Normal line, just copy
            outf << line << "\n";
            continue;
        }

        // ---------------- MACRO EXPANSION ----------------
        MNTEntry curr = MNT[macroIndex];
        vector<string> actualParams;

        // Split actual parameters
        string param;
        while (getline(ss, param, ',')) {
            param.erase(0, param.find_first_not_of(" \t"));
            param.erase(param.find_last_not_of(" \t") + 1);
            if (!param.empty())
                actualParams.push_back(param);
        }

        // Create Actual Parameter Table (APTAB)
        map<int, string> APTAB;
        int kpIndex = curr.kpdtIndex - 1;

        // Fill keyword params with default
        for (int i = 0; i < curr.kp; i++) {
            APTAB[curr.pp + i + 1] = KPDTAB[kpIndex + i].value;
        }

        // Fill positional parameters
        for (int i = 0; i < actualParams.size(); i++) {
            APTAB[i + 1] = actualParams[i];
        }

        // Expand macro from MDT
        int mdti = curr.mdtIndex - 1;
        while (MDT[mdti] != "MEND") {
            string expanded = MDT[mdti];

            // Replace (P,n) with actual value from APTAB
            size_t pos = expanded.find("(P,");
            while (pos != string::npos) {
                int num = expanded[pos + 3] - '0';
                string value = APTAB[num];
                expanded.replace(pos, 5, value);
                pos = expanded.find("(P,", pos + 1);
            }

            outf << expanded << "\n";
            mdti++;
        }
    }

    cout << " Pass-II Macro Expansion Completed.\nOutput generated in output.txt\n";
    return 0;
}
