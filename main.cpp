#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>

using namespace std;

// Store created tables and their data
unordered_set<string> createdTables;
unordered_map<string, vector<string>> tableData;

// Function prototypes
void createFile(const string& filename);
void createTable(const string& line);
void insertIntoTable(const string& line);
void displayTables();
void processLine(const string& line, const string& filename);
void displayDatabasePath(const string& filename);

void createFile(const string& filename) {
    ofstream outfile(filename);
    if (outfile) {
        cout << "File created: " << filename << endl;
    } else {
        cout << "Error creating file: " << filename << endl;
    }
}

void createTable(const string& line) {
    size_t pos = line.find("TABLE") + 6;  // Get position after "CREATE TABLE "
    size_t endPos = line.find('(', pos); // Find position of '('
    string tableName = line.substr(pos, endPos - pos); // Extract table name
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    if (createdTables.find(tableName) == createdTables.end()) {
        createdTables.insert(tableName);
        cout << "Table created: " << tableName << endl;
    } else {
        cout << "Table already exists: " << tableName << endl;
    }
}

void insertIntoTable(const string& line) {
    size_t pos = line.find("INSERT INTO") + 11; // Position after "INSERT INTO "
    size_t tableEnd = line.find('(', pos);      // End of table name
    string tableName = line.substr(pos, tableEnd - pos);
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    if (createdTables.find(tableName) == createdTables.end()) {
        cout << "Table does not exist: " << tableName << endl;
        return;
    }

    size_t valuesPos = line.find("VALUES", tableEnd) + 6; // Position after "VALUES"
    string values = line.substr(valuesPos);
    values.erase(remove_if(values.begin(), values.end(), ::isspace), values.end());

    tableData[tableName].push_back(values);
    cout << "Inserted into table " << tableName << ": " << values << endl;
}

void displayTables() {
    if (createdTables.empty()) {
        cout << "No tables have been created." << endl;
    } else {
        cout << "Tables:" << endl;
        for (const string& tableName : createdTables) {
            cout << "- " << tableName << endl;
        }
    }
}

void displayDatabasePath(const string& filename) {
    cout << "Path of the .mdb file: " << filename << endl;
}

void processLine(const string& line, const string& filename) {
    // Print each line
    cout << line << endl;

    if (line.find("CREATE") != string::npos) {
        if (line.find("CREATE fileOutput1.txt") != string::npos) {
            createFile("fileOutput1.txt");
        } else if (line.find("CREATE TABLE") != string::npos) {
            createTable(line);
        }
    } else if (line.find("DATABASES") != string::npos) {
        displayDatabasePath(filename);
    } else if (line.find("TABLES") != string::npos) {
        displayTables();
    } else if (line.find("INSERT INTO") != string::npos) {
        insertIntoTable(line);
    }
}

int main() {
    string basePath = "C:\\Users\\User\\Projects\\Light_Mariadb_Interpreter\\data\\";
    string fileInput;

    cout << "Enter the .mdb file name: ";
    getline(cin, fileInput);
    string filename = basePath + fileInput;

    ifstream infile(filename);

    if (infile.fail()) {
        cout << "Error opening file: " << filename << endl;
    } else {
        string line;
        while (getline(infile, line)) {
            processLine(line, filename);
        }
        infile.close();
    }

    return 0;
}
