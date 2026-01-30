#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>

using namespace std;

// Structure to hold movie information
struct Movie {
    string date;
    string name;
    string year;
    string letterboxdURI;
    string rating;
    string rewatch;
    string tags;
    string watchedDate;
};

// Function to trim whitespace from string
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n\"");
    if (string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n\"");
    return str.substr(first, (last - first + 1));
}

// Function to parse a CSV line (handles quoted fields with commas)
vector<string> parseCSVLine(const string& line) {
    vector<string> fields;
    string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];

        if (c == '"') {
            inQuotes = !inQuotes;
        }
        else if (c == ',' && !inQuotes) {
            fields.push_back(trim(field));
            field.clear();
        }
        else {
            field += c;
        }
    }

    // Add the last field
    fields.push_back(trim(field));

    return fields;
}

// Safe string to double conversion
double safeStringToDouble(const string& str) {
    if (str.empty()) return 0.0;

    try {
        size_t idx;
        double value = stod(str, &idx);
        // Check if entire string was converted
        if (idx != str.length()) {
            return 0.0;
        }
        return value;
    }
    catch (...) {
        return 0.0;
    }
}

// Function to read and parse the Letterboxd diary CSV
vector<Movie> readLetterboxdCSV(const string& filename) {
    vector<Movie> movies;

    // Try to open file with different methods
    ifstream file;

    // Try opening as-is
    file.open(filename);

    // If that fails, try with binary mode
    if (!file.is_open()) {
        file.open(filename, ios::binary);
    }

    if (!file.is_open()) {
        cerr << "Error: Could not open file '" << filename << "'" << endl;
        cerr << "Please check that:" << endl;
        cerr << "  - The file path is correct" << endl;
        cerr << "  - The file exists" << endl;
        cerr << "  - You have permission to read the file" << endl;
        return movies;
    }

    string line;
    int lineNumber = 0;
    bool isFirstLine = true;

    while (getline(file, line)) {
        lineNumber++;

        // Remove any carriage return characters
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }

        // Skip header line
        if (isFirstLine) {
            isFirstLine = false;
            cout << "CSV Header: " << line << endl << endl;
            continue;
        }

        // Skip empty lines
        if (line.empty()) continue;

        try {
            vector<string> fields = parseCSVLine(line);

            // Debug: show how many fields we found
            if (lineNumber == 2) {
                cout << "First data line has " << fields.size() << " fields" << endl << endl;
            }

            // Letterboxd diary.csv format:
            // Date,Name,Year,Letterboxd URI,Rating,Rewatch,Tags,Watched Date
            if (fields.size() >= 2) {  // At minimum we need date and name
                Movie movie;
                movie.date = fields.size() > 0 ? fields[0] : "";
                movie.name = fields.size() > 1 ? fields[1] : "";
                movie.year = fields.size() > 2 ? fields[2] : "";
                movie.letterboxdURI = fields.size() > 3 ? fields[3] : "";
                movie.rating = fields.size() > 4 ? fields[4] : "";
                movie.rewatch = fields.size() > 5 ? fields[5] : "";
                movie.tags = fields.size() > 6 ? fields[6] : "";
                movie.watchedDate = fields.size() > 7 ? fields[7] : "";

                movies.push_back(movie);
            }
        }
        catch (const exception& e) {
            cerr << "Warning: Error parsing line " << lineNumber << ": " << e.what() << endl;
            continue;
        }
        catch (...) {
            cerr << "Warning: Unknown error parsing line " << lineNumber << endl;
            continue;
        }
    }

    file.close();

    cout << "Successfully read " << movies.size() << " movies from CSV" << endl << endl;

    return movies;
}

// Function to convert rating to stars
string ratingToStars(const string& rating) {
    if (rating.empty()) return "";

    double ratingValue = safeStringToDouble(rating);

    // Convert 0-5 scale to star display
    if (ratingValue == 0.0) return "";

    int fullStars = (int)ratingValue;
    bool halfStar = (ratingValue - fullStars) >= 0.5;

    string stars;
    for (int i = 0; i < fullStars; i++) {
        stars += "*";
    }
    if (halfStar) {
        stars += "½";
    }

    return stars + " (" + rating + "/5)";
}

// Function to open file dialog (Windows only)
string openFileDialog() {
    char filename[MAX_PATH] = "";

    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "CSV Files (*.csv)\0*.csv\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "csv";
    ofn.lpstrTitle = "Select Letterboxd diary.csv file";

    if (GetOpenFileNameA(&ofn)) {
        return string(filename);
    }

    return "";
}

int main() {
    // Wrap everything in try-catch to prevent crashes
    try {
        cout << "========================================" << endl;
        cout << "  Letterboxd CSV Export Reader" << endl;
        cout << "========================================" << endl;
        cout << endl;
        cout << "Instructions:" << endl;
        cout << "1. Log into Letterboxd.com" << endl;
        cout << "2. Go to Settings > Import & Export" << endl;
        cout << "3. Click 'Export Your Data'" << endl;
        cout << "4. Extract the ZIP file" << endl;
        cout << "5. Use the 'diary.csv' file below" << endl;
        cout << endl;
        cout << "========================================" << endl;
        cout << endl;

        cout << "Choose an option:" << endl;
        cout << "1. Browse for diary.csv file" << endl;
        cout << "2. Enter file path manually" << endl;
        cout << endl;
        cout << "Enter choice (1 or 2): ";

        string choice;
        getline(cin, choice);

        string filename;

        if (choice == "1") {
            cout << endl << "Opening file browser..." << endl;
            filename = openFileDialog();

            if (filename.empty()) {
                cout << "No file selected." << endl;
                cout << endl << "Press Enter to exit...";
                cin.get();
                return 0;
            }
        }
        else if (choice == "2") {
            cout << endl << "Enter the full path to diary.csv" << endl;
            cout << "(Tip: You can drag and drop the file into this window)" << endl;
            cout << "Path: ";
            getline(cin, filename);

            // Remove quotes if user dragged and dropped
            if (!filename.empty() && filename[0] == '"') {
                filename = filename.substr(1, filename.length() - 2);
            }
        }
        else {
            cout << "Invalid choice." << endl;
            cout << endl << "Press Enter to exit...";
            cin.get();
            return 0;
        }

        cout << endl << "Reading file: " << filename << endl << endl;

        vector<Movie> movies = readLetterboxdCSV(filename);

        if (movies.empty()) {
            cout << "========================================" << endl;
            cout << "No movies found or error reading file." << endl;
            cout << "========================================" << endl;
            cout << endl;
            cout << "Troubleshooting tips:" << endl;
            cout << "- Make sure you selected 'diary.csv' (not 'watched.csv' or other files)" << endl;
            cout << "- Check that the file isn't empty" << endl;
            cout << "- Try extracting the ZIP file again" << endl;
            cout << endl << "Press Enter to exit...";
            cin.get();
            return 0;
        }

        cout << "========================================" << endl;
        cout << "Found " << movies.size() << " watched movies!" << endl;
        cout << "========================================" << endl << endl;

        // Option to display in different orders
        cout << "How would you like to view your movies?" << endl;
        cout << "1. Most recent first (default)" << endl;
        cout << "2. Oldest first" << endl;
        cout << "3. Alphabetically by title" << endl;
        cout << "4. Highest rated first" << endl;
        cout << endl;
        cout << "Enter choice (1-4) or press Enter for default: ";

        string sortChoice;
        getline(cin, sortChoice);

        // Sort movies based on choice
        if (sortChoice == "2") {
            // Reverse for oldest first
            reverse(movies.begin(), movies.end());
        }
        else if (sortChoice == "3") {
            sort(movies.begin(), movies.end(), [](const Movie& a, const Movie& b) {
                return a.name < b.name;
                });
        }
        else if (sortChoice == "4") {
            sort(movies.begin(), movies.end(), [](const Movie& a, const Movie& b) {
                double ratingA = safeStringToDouble(a.rating);
                double ratingB = safeStringToDouble(b.rating);
                return ratingA > ratingB;
                });
        }
        // else: default is most recent first (already in that order)

        cout << endl << "========================================" << endl << endl;

        // Display all movies
        for (size_t i = 0; i < movies.size(); i++) {
            cout << (i + 1) << ". " << movies[i].name;

            if (!movies[i].year.empty()) {
                cout << " (" << movies[i].year << ")";
            }

            cout << endl;

            if (!movies[i].watchedDate.empty()) {
                cout << "   Watched: " << movies[i].watchedDate << endl;
            }
            else if (!movies[i].date.empty()) {
                cout << "   Watched: " << movies[i].date << endl;
            }

            if (!movies[i].rating.empty()) {
                string stars = ratingToStars(movies[i].rating);
                if (!stars.empty()) {
                    cout << "   Rating: " << stars << endl;
                }
            }

            if (!movies[i].rewatch.empty() && movies[i].rewatch != "No") {
                cout << "   [REWATCH]" << endl;
            }

            if (!movies[i].tags.empty()) {
                cout << "   Tags: " << movies[i].tags << endl;
            }

            cout << endl;
        }

        cout << "========================================" << endl;
        cout << "Total movies watched: " << movies.size() << endl;

        // Calculate some statistics
        int ratedMovies = 0;
        double totalRating = 0.0;
        int rewatchCount = 0;

        for (const auto& movie : movies) {
            if (!movie.rating.empty()) {
                double rating = safeStringToDouble(movie.rating);
                if (rating > 0.0) {
                    ratedMovies++;
                    totalRating += rating;
                }
            }
            if (!movie.rewatch.empty() && movie.rewatch != "No") {
                rewatchCount++;
            }
        }

        if (ratedMovies > 0) {
            double avgRating = totalRating / ratedMovies;
            cout << "Average rating: ";
            cout.precision(2);
            cout << fixed << avgRating << "/5 (based on " << ratedMovies << " rated films)" << endl;
        }

        if (rewatchCount > 0) {
            cout << "Rewatches: " << rewatchCount << endl;
        }

        cout << endl << "Press Enter to exit...";
        cin.get();

    }
    catch (const exception& e) {
        cerr << endl << "ERROR: " << e.what() << endl;
        cerr << endl << "Press Enter to exit...";
        cin.get();
        return 1;
    }
    catch (...) {
        cerr << endl << "ERROR: An unknown error occurred" << endl;
        cerr << endl << "Press Enter to exit...";
        cin.get();
        return 1;
    }

    return 0;
}