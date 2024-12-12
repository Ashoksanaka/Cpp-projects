#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

// Function to load ratings matrix from CSV
void loadRatings(const string& filename, vector<vector<int>>& ratings, vector<string>& users, vector<string>& movies) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        exit(1);
    }

    string line;
    bool firstLine = true;

    // Read the CSV line by line
    while (getline(file, line)) {
        stringstream ss(line);
        string cell;

        if (firstLine) {
            // First line is the movie names
            while (getline(ss, cell, ',')) {
                if (!cell.empty()) {
                    movies.push_back(cell);  // Populate the list of movies
                }
            }
            firstLine = false;
        }
        else {
            // Read user ratings
            vector<int> userRatings;
            string user;

            // Read the user name (first column)
            getline(ss, user, ',');
            users.push_back(user);

            // Read ratings for each movie
            while (getline(ss, cell, ',')) {
                int rating = 0;
                if (!cell.empty()) {
                    rating = stoi(cell);  // Convert to int
                }
                userRatings.push_back(rating);
            }
            ratings.push_back(userRatings);
        }
    }

    file.close();
}

// Function to display all ratings given by the user (only rated movies)
void displayRatedMovies(const vector<vector<int>>& ratings, const vector<string>& users, const vector<string>& movies, const string& userName) {
    // Find the index of the user
    auto it = find(users.begin(), users.end(), userName);
    if (it == users.end()) {
        cout << "User " << userName << " not found!" << endl;
        return;
    }
    int userIndex = distance(users.begin(), it);

    // Display ratings given by the user (only rated movies)
    cout << "\nRated movies of " << userName << ":\n";
    bool hasRatedMovies = false;

    // Iterate through all movies and check if the user has rated them
    for (size_t i = 0; i < ratings[userIndex].size(); ++i) {
        if (ratings[userIndex][i] != 0) {  // Only display rated movies (rating > 0)
            cout << movies[i] << " -> Rating: " << ratings[userIndex][i] << endl;
            hasRatedMovies = true;
        }
    }

    if (!hasRatedMovies) {
        cout << "No rated movies for this user.\n";
    }
}

// Function to calculate cosine similarity between two users
double cosineSimilarity(const vector<int>& user1, const vector<int>& user2) {
    int dotProduct = 0;
    int norm1 = 0;
    int norm2 = 0;
    int commonRatings = 0;

    for (size_t i = 0; i < user1.size(); ++i) {
        if (user1[i] != 0 && user2[i] != 0) {  // Only consider rated movies
            dotProduct += user1[i] * user2[i];
            norm1 += user1[i] * user1[i];
            norm2 += user2[i] * user2[i];
            commonRatings++;
        }
    }

    if (commonRatings == 0 || norm1 == 0 || norm2 == 0) {
        return 0.0;  // No similarity or zero norm, return similarity of 0
    }

    return dotProduct / (sqrt(norm1) * sqrt(norm2));
}

// Function to predict the rating for a movie using weighted average of similar users' ratings
double predictRating(const vector<vector<int>>& ratings, const vector<vector<double>>& similarities, size_t userIndex, size_t movieIndex) {
    double weightedSum = 0;
    double similaritySum = 0;

    for (size_t i = 0; i < ratings.size(); ++i) {
        if (ratings[i][movieIndex] != 0) {  // Only consider rated movies
            weightedSum += similarities[userIndex][i] * ratings[i][movieIndex];
            similaritySum += abs(similarities[userIndex][i]);
        }
    }

    if (similaritySum == 0) {
        return 0.0;  // No similarity found for prediction
    }

    return weightedSum / similaritySum;
}

// Function to display predicted ratings for unrated movies
void displayPredictedRatings(const vector<vector<int>>& ratings, const vector<vector<double>>& similarities, const string& userName, const vector<string>& users, const vector<string>& movies) {
    // Find the index of the user
    auto it = find(users.begin(), users.end(), userName);
    if (it == users.end()) {
        cout << "User " << userName << " not found!" << endl;
        return;
    }
    int userIndex = distance(users.begin(), it);

    // Display predicted ratings for unrated movies
    for (size_t i = 0; i < ratings[0].size(); ++i) {
        if (ratings[userIndex][i] == 0) {  // Only display unrated movies
            double predictedRating = predictRating(ratings, similarities, userIndex, i);
            cout << movies[i] << ": " << (int)predictedRating << endl;
        }
    }
}

// Function to display top N recommended movies
void displayTopNRecommendations(const vector<vector<int>>& ratings, const vector<vector<double>>& similarities, const string& userName, int N, const vector<string>& users, const vector<string>& movies) {
    // Find the index of the user
    auto it = find(users.begin(), users.end(), userName);
    if (it == users.end()) {
        cout << "User " << userName << " not found!" << endl;
        return;
    }
    int userIndex = distance(users.begin(), it);

    vector<pair<int, double>> recommendations;

    // First, recommend unrated movies based on predicted ratings
    for (size_t i = 0; i < ratings[0].size(); ++i) {  // For each movie
        if (ratings[userIndex][i] == 0) {  // Only recommend unrated movies
            double predictedRating = predictRating(ratings, similarities, userIndex, i);
            recommendations.push_back(make_pair(i, predictedRating));
        }
    }

    // Then, add rated movies to the recommendations list based on predicted ratings or similarity
    for (size_t i = 0; i < ratings[0].size(); ++i) {
        if (ratings[userIndex][i] != 0) {  // Include rated movies
            double predictedRating = predictRating(ratings, similarities, userIndex, i);
            recommendations.push_back(make_pair(i, predictedRating));
        }
    }

    // Sort recommendations by predicted rating (in descending order)
    sort(recommendations.begin(), recommendations.end(), [](const pair<int, double>& a, const pair<int, double>& b) {
        return a.second > b.second;
        });

    // Adjust number of recommendations to be at most N (even if there are fewer than N unrated and rated movies)
    if (recommendations.size() < N) {
        N = recommendations.size();  // Adjust N if fewer than N unrated and rated movies
    }

    // Display top N recommended movies
    for (int i = 0; i < N; ++i) {
        cout << "Rank " << i + 1 << ": " << movies[recommendations[i].first] << " with predicted rating: " << (int)recommendations[i].second << endl;
    }
}

int main() {
    vector<string> users;
    vector<string> movies;
    vector<vector<int>> ratings;
    string filename = "<File name>";  // Ensure the path is correct

    // Load the ratings matrix from a CSV file
    loadRatings(filename, ratings, users, movies);

    // Check if users are loaded
    cout << "Loaded users: ";
    for (const auto& user : users) {
        cout << user << " ";
    }
    cout << endl;

    // Ask user for the name of the user for predictions and recommendations
    string userName;
    cout << "Enter the user name for predictions and recommendations: ";
    getline(cin, userName);  // Ensure user name input is handled properly

    // Display the rated movies for the given user
    displayRatedMovies(ratings, users, movies, userName);

    // Compute similarities between all users
    vector<vector<double>> similarities(ratings.size(), vector<double>(ratings.size(), 0));
    for (size_t i = 0; i < ratings.size(); ++i) {
        for (size_t j = i + 1; j < ratings.size(); ++j) {
            double similarity = cosineSimilarity(ratings[i], ratings[j]);
            similarities[i][j] = similarity;
            similarities[j][i] = similarity;  // Ensure symmetry
        }
    }

    // Display predicted ratings for unrated movies
    cout << "\nPredicted ratings for unrated movies of " << userName << ":\n";
    displayPredictedRatings(ratings, similarities, userName, users, movies);

    // Display top N recommended movies
    int N = 5;  // Number of top movies to recommend (can be adjusted)
    cout << "\nTop " << N << " recommended movies for " << userName << ":\n";
    displayTopNRecommendations(ratings, similarities, userName, N, users, movies);

    return 0;
}
