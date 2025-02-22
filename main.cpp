#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <algorithm>


using namespace std;


// Function prototypes
vector<string> parseCSV(const string& row);
string join(const vector<string>& elements, const string& delimiter);
size_t getColumnIndex(const string& tableName, const string& columnName);
void createFile(const string& filename);
void createTable(const string& line);
void insertIntoTable(const string& line);
void displayTables();
void processLine(const string& line, const string& filename);
void displayDatabasePath(const string& filename);
void writeToOutputFile(const string& message);
void processDeleteData(const string& deleteCommand);
void processUpdateData(const string& updateCommand);


// Parses a CSV row into individual values (strings)
// Strips unwanted characters like spaces, quotes, and parentheses
vector<string> parseCSV(const string& row) {
    vector<string> result;
    stringstream ss(row);
    string value;

    while (getline(ss, value, ',')) {
        // Remove surrounding spaces and quotes
        value.erase(remove(value.begin(), value.end(), ' '), value.end());

        // Remove any surrounding quotes
        if (value.front() == '\'' || value.front() == '\"') {
            value = value.substr(1, value.length() - 2);
        }

        // Remove any unwanted parentheses or special characters
        if (value.front() == '(' || value.back() == ')') {
            value = value.substr(1, value.length() - 2);  // Remove parentheses
        }

        if (value.empty()) {
            writeToOutputFile("Warning: Empty value found in CSV row.");
        }

        result.push_back(value);
    }
    return result;
}


// Joins elements of a vector into a single string with a specified delimiter
string join(const vector<string>& elements, const string& delimiter) {
    string result;
    for (size_t i = 0; i < elements.size(); ++i) {
        result += elements[i];
        if (i < elements.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}


ofstream outputFile;  // Output file
unordered_set<string> createdTables; // Created tables
unordered_map<string, vector<string>> tableData; // Map of table data
unordered_map<string, vector<string>> tableSchemas = { // Use for update comma
    {"friends", {"friend_id", "first_name", "last_name", "email", "phone_number", "birthday", "star_sign", "address", "next_met", "mutual_friends"}},
    {"customer", {"customer_id", "customer_name", "customer_city", "customer_state", "customer_country", "customer_phone", "customer_email"}}
};



// Gets the index of a column within a table schema
size_t getColumnIndex(const string& tableName, const string& columnName) {
    if (tableSchemas.find(tableName) == tableSchemas.end()) {
        writeToOutputFile("No schema found for table: " + tableName);
        return string::npos;
    }

    const auto& schema = tableSchemas[tableName];
    auto columnIt = find(schema.begin(), schema.end(), columnName);
    if (columnIt == schema.end()) {
        writeToOutputFile("Column " + columnName + " not found in schema for table " + tableName);
        return string::npos;
    }

    return distance(schema.begin(), columnIt);
}


// Writes messages to the output file and console
void writeToOutputFile(const string& message) {
    if (!message.empty()) {  // Only write if the message is not empty
        cout << message << endl;  // Display on screen
        if (outputFile.is_open()) {
            outputFile << message << endl;  // Write to file
        }
    }
}

// Creates a new file for output
void createFile(const string& filename) {
    outputFile.open(filename);
    if (!outputFile.is_open()) {
        writeToOutputFile("Error creating or opening file: " + filename);
    } else {
        writeToOutputFile(">CREATE " + filename + ";");
    }
}


// Creates a new table based on a CREATE TABLE command
void createTable(const string& line) {
    size_t pos = line.find("TABLE") + 6;  // Get position after "CREATE TABLE "
    size_t endPos = line.find('(', pos); // Find position of '('
    string tableName = line.substr(pos, endPos - pos); // Extract table name
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    if (createdTables.find(tableName) == createdTables.end()) {
        createdTables.insert(tableName);
    } else {
        writeToOutputFile("Table already exists: " + tableName);
    }
}



// Inserts data into a table based on an INSERT INTO command
void insertIntoTable(const string& line) {
    size_t pos = line.find("INSERT INTO") + 11;
    size_t tableEnd = line.find('(', pos);

    if (tableEnd == string::npos) {
        writeToOutputFile("Invalid INSERT command syntax.");
        return;
    }

    string tableName = line.substr(pos, tableEnd - pos);
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    if (createdTables.find(tableName) == createdTables.end()) {
        writeToOutputFile("Table does not exist: " + tableName);
        return;
    }

    size_t valuesPos = line.find("VALUES", tableEnd) + 6; // Position after "VALUES"
    string values = line.substr(valuesPos);
    values.erase(remove_if(values.begin(), values.end(), ::isspace), values.end());

    // Remove parentheses and quotes around values
    if (values.front() == '(' && values.back() == ')') {
        values = values.substr(1, values.length() - 2); // Remove outer parentheses
    }

    // Remove single quotes around each value
    size_t posStart = 0;
    while ((posStart = values.find('\'', posStart)) != string::npos) {
        values.erase(posStart, 1);  // Remove single quotes
    }

    // Remove semicolon at the end if it exists
    if (!values.empty() && values.back() == ';') {
        values.pop_back();  // Remove the semicolon
    }

    tableData[tableName].push_back(values);  // Store the row without parentheses, quotes, or semicolon
}


// Displays the names of all created tables
void displayTables() {
    if (createdTables.empty()) {
        writeToOutputFile("No tables have been created.");
    } else {
        for (const string& tableName : createdTables) {
            writeToOutputFile(tableName);
        }
    }
}


// Displays the database file path
void displayDatabasePath(const string& filename) {
    writeToOutputFile(filename);
}


// Processes DELETE command to remove rows from a tabl
void processDeleteData(const string& deleteCommand) {
    // Extract the table name from the DELETE command
    size_t deletePos = deleteCommand.find("DELETE FROM ");
    size_t wherePos = deleteCommand.find("WHERE");

    if (deletePos == string::npos || wherePos == string::npos) {
        writeToOutputFile("Invalid DELETE command.");
        return;
    }

    // Extract the table name
    string tableName = deleteCommand.substr(deletePos + 12, wherePos - deletePos - 12);
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

    // Check if the table exists
    if (createdTables.find(tableName) == createdTables.end()) {
        writeToOutputFile("> Invalid table name: " + tableName);
        return;
    }

    // Extract the WHERE condition
    string whereClause = deleteCommand.substr(wherePos + 6); // After "WHERE "
    whereClause.erase(remove_if(whereClause.begin(), whereClause.end(), ::isspace), whereClause.end());

    size_t whereEqualPos = whereClause.find('=');
    if (whereEqualPos == string::npos) {
        writeToOutputFile("Invalid WHERE condition in DELETE command.");
        return;
    }

    string whereColumn = whereClause.substr(0, whereEqualPos);
    string whereValue = whereClause.substr(whereEqualPos + 1);
    whereValue.erase(remove(whereValue.begin(), whereValue.end(), ';'), whereValue.end());

    // Iterate through each row in the table and delete if the condition matches
    bool found = false;
    for (auto it = tableData[tableName].begin(); it != tableData[tableName].end();) {
        string row = *it;

        // Extract the value of the first column (assuming it's the condition column)
        size_t posComma = row.find(',');
        string valueBeforeComma = row.substr(0, posComma);

        // Clean up the value to remove parentheses or unwanted characters
        valueBeforeComma.erase(remove_if(valueBeforeComma.begin(), valueBeforeComma.end(), [](char c) {
            return !isalnum(c);}), valueBeforeComma.end()
        );

        if (valueBeforeComma == whereValue) {
            it = tableData[tableName].erase(it); // Remove the row
            found = true;
        } else {
            ++it; // Move to the next row
        }
    }

    if (!found) {
        writeToOutputFile("No matching row found for " + whereColumn + "=" + whereValue);
    }
}


// Processes UPDATE command to modify rows in a table
void processUpdateData(const string& updateCommand) {
    if (updateCommand.empty()) {
        writeToOutputFile("Error: Empty UPDATE command.");
        return;
    }

    // Find the position of SET and WHERE in the updateCommand
    size_t setPos = updateCommand.find("SET ");
    size_t wherePos = updateCommand.find("WHERE");


    if (setPos == string::npos || wherePos == string::npos) {
        writeToOutputFile("Invalid UPDATE command.");
        return;
    }

    // Extract the table name (assumed to be after "UPDATE ")
    string tableName = updateCommand.substr(6, setPos - 6);
    tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end()); // Remove spaces

    // Validate the table name
    if (tableData.find(tableName) == tableData.end()) {
        writeToOutputFile("Invalid table name: " + tableName);
        return;
    }

    // Extract the SET clause (e.g., column_name='new_value')
    string setClause = updateCommand.substr(setPos + 4, wherePos - setPos - 4);
    size_t equalPos = setClause.find('=');
    if (equalPos == string::npos) {
        writeToOutputFile("Invalid SET clause in UPDATE command.");
        return;
    }

    // Extract the column to update and its new value
    string columnToUpdate = setClause.substr(0, equalPos);
    string newValue = setClause.substr(equalPos + 1);
    columnToUpdate.erase(remove_if(columnToUpdate.begin(), columnToUpdate.end(), ::isspace), columnToUpdate.end());
    newValue.erase(remove_if(newValue.begin(), newValue.end(), ::isspace), newValue.end());

    // Extract the WHERE condition (e.g., column_name=value)
    string wherePart = updateCommand.substr(wherePos + 6);  // After "WHERE "
    size_t whereEqualPos = wherePart.find('=');
    if (whereEqualPos == string::npos) {
        writeToOutputFile("Invalid WHERE condition in UPDATE command.");
        return;
    }

    string whereColumn = wherePart.substr(0, whereEqualPos);
    string whereValue = wherePart.substr(whereEqualPos + 1);
    whereColumn.erase(remove_if(whereColumn.begin(), whereColumn.end(), ::isspace), whereColumn.end());
    whereValue.erase(remove(whereValue.begin(), whereValue.end(), ';'), whereValue.end());
    whereValue.erase(remove_if(whereValue.begin(), whereValue.end(), ::isspace), whereValue.end());

    // Get the column index to update
    size_t columnToUpdateIndex = getColumnIndex(tableName, columnToUpdate);
    if (columnToUpdateIndex == string::npos) return;

    // Get the WHERE column index
    size_t whereColumnIndex = getColumnIndex(tableName, whereColumn);
    if (whereColumnIndex == string::npos) return;

    // Process rows for the specified table
    bool found = false;
    for (auto& row : tableData[tableName]) {
        // Remove parentheses from the row data
        row.erase(remove(row.begin(), row.end(), '('), row.end());
        row.erase(remove(row.begin(), row.end(), ')'), row.end());

        // Split the row into columns
        vector<string> columns;
        size_t startPos = 0, commaPos = 0;
        while ((commaPos = row.find(',', startPos)) != string::npos) {
            columns.push_back(row.substr(startPos, commaPos - startPos));
            startPos = commaPos + 1;
        }
        columns.push_back(row.substr(startPos)); // Add the last column


        // Check if the WHERE condition matches (using the column index)
        if (columns[whereColumnIndex] == whereValue) {

            // Update the column based on the column index
            columns[columnToUpdateIndex] = newValue;

            // Rebuild the row with the updated value
            string updatedRow = columns[0];
            for (size_t i = 1; i < columns.size(); ++i) {
                updatedRow += "," + columns[i];
            }

            // Replace the old row with the updated one
            row = updatedRow;
            found = true;
            break; // Update only the first matching row
        }
    }

    if (!found) {
        writeToOutputFile("No matching row found for WHERE condition: " + whereColumn + "=" + whereValue);
    }
}

// Read command
void processLine(const string& line, const string& filename) {
    // Skip empty or whitespace-only lines
    if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) {
        return;  // Skip this line
    }

    // Add '>' before the commands like CREATE, INSERT, DATABASES, etc.
    string modifiedLine = line;
    bool isCommand = false;

    if (modifiedLine.find("CREATE") != string::npos ||
        modifiedLine.find("DATABASES") != string::npos ||
        modifiedLine.find("INSERT INTO") != string::npos ||
        modifiedLine.find("SELECT") != string::npos ||
        modifiedLine.find("UPDATE") != string::npos ||
        modifiedLine.find("DELETE") != string::npos ||
        modifiedLine.find("TABLES") != string::npos) {
        modifiedLine = ">" + modifiedLine;
        isCommand = true;
    }

    // If it's a command, print it with '>'
    writeToOutputFile(modifiedLine);

    // Handle different commands as before
    if (line.find("CREATE") != string::npos) {
        // Check if it's CREATE FILE or CREATE TABLE
        size_t pos = line.find("CREATE") + 6;
        size_t spacePos = line.find(' ', pos);
        string fileName = line.substr(spacePos + 1);
        fileName.erase(remove_if(fileName.begin(), fileName.end(), ::isspace), fileName.end());

        // Remove semicolon if it exists at the end of the fileName
        if (!fileName.empty() && fileName.back() == ';') {
            fileName.pop_back();  // Removes the semicolon
        }

        // If it's creating a file (not a table), just create the file
        if (line.find("TABLE") == string::npos) {
            createFile(fileName);  // Now it passes without the semicolon
        }

        else {
            createTable(line);
        }
    }

    else if (line.find("DATABASES") != string::npos) {
        displayDatabasePath(filename);
    }

    else if (line.find("TABLES") != string::npos) {
        displayTables();
    }

    else if (line.find("INSERT INTO") != string::npos) {
        insertIntoTable(line);
    }

    else if (line.find("SELECT COUNT(*) FROM") != string::npos) {
        string tableName = line.substr(20, line.find(';') - 20);
        tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

        if (createdTables.find(tableName) == createdTables.end()) {
            writeToOutputFile("> Table does not exist: " + tableName);
        }

        else {
            size_t rowCount = tableData[tableName].size();

            // Write the count without the '>' symbol
            writeToOutputFile(to_string(rowCount));  // Directly write the count without ">"
        }
    }

    else if (line.find("SELECT * FROM") != string::npos) {
        string tableName = line.substr(14, line.find(';') - 14);
        tableName.erase(remove_if(tableName.begin(), tableName.end(), ::isspace), tableName.end());

        if (tableName.empty()) {
            writeToOutputFile("> No table name specified. Available tables and their schemas:");
            for (const auto& [name, schema] : tableSchemas) {
                writeToOutputFile("> " + name + ": " + join(schema, ", "));
            }
        }

        else if (createdTables.find(tableName) == createdTables.end()) {
            writeToOutputFile("> Table does not exist: " + tableName);
        }

        else {
            // Check table name and output corresponding columns
            if (tableName == "customer") {
                writeToOutputFile("customer_id, name, email, phone, address");
            }

            else if (tableName == "friends") {
                writeToOutputFile("friend_id, first_name, last_name, email, phone_number, birthday, star_sign, address, next_met, mutual_friends");
            }
            // Add more table-specific conditions here if needed

            // Process rows for the specified table
            for (const string& row : tableData[tableName]) {
                string cleanedRow = row;

                // Remove parentheses
                cleanedRow.erase(remove(cleanedRow.begin(), cleanedRow.end(), '('), cleanedRow.end());
                cleanedRow.erase(remove(cleanedRow.begin(), cleanedRow.end(), ')'), cleanedRow.end());

                // Remove single quotes from the values
                size_t posStart = 0;
                while ((posStart = cleanedRow.find('\'', posStart)) != string::npos) {
                    cleanedRow.erase(posStart, 1);  // Remove single quotes
                }

                writeToOutputFile(cleanedRow);  // Display data without '>'
            }
        }
    }

     else if (line.find("DELETE FROM") != string::npos) {
        processDeleteData(line);  // Pass 'line' directly as an argument
    }

    else if (line.find("UPDATE") != string::npos) {
        processUpdateData(line);  // Pass the UPDATE command directly to the function
    }
}

int main() {
    string basePath = "C:\\Users\\User\\Projects\\Light_Mariadb_Interpreter\\data\\";  // Base directory
    string fileInput;

    getline(cin, fileInput);
    string filename = basePath + fileInput;

    ifstream infile(filename);

    if (infile.fail()) {
        writeToOutputFile("Error opening file: " + filename);
    }

    else {
        string line;
        while (getline(infile, line)) {
            processLine(line, filename);
        }
        infile.close();
    }

    if (outputFile.is_open()) {
        outputFile.close();  // Close the output file after processing
    }

    return 0;
}
