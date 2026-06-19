#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Callback function to write response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Fetch weather data from API
std::string fetchWeather(const std::string& city) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        // Using wttr.in - free weather API, no key needed
        std::string url = "https://wttr.in/" + city + "?format=j1";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "weather-cli/1.0");
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        // Set CA cert path for OpenSSL
        curl_easy_setopt(curl, CURLOPT_CAPATH, "C:/msys64/mingw64/etc/ssl/certs");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "C:/msys64/mingw64/etc/ssl/certs/ca-bundle.crt");

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            return "Error: " + std::string(curl_easy_strerror(res));
        }
    }
    return readBuffer;
}

// Parse and display weather information
void displayWeather(const std::string& city, const std::string& response) {
    try {
        auto data = json::parse(response);

        // Current conditions
        auto current = data["current_condition"][0];
        std::string tempC = current["temp_C"];
        std::string tempF = current["temp_F"];
        std::string humidity = current["humidity"];
        std::string windSpeed = current["windspeedKmph"];
        std::string description = current["weatherDesc"][0]["value"];

        std::cout << "\n===== Weather for " << city << " =====\n";
        std::cout << "Temperature: " << tempC << "°C (" << tempF << "°F)\n";
        std::cout << "Condition:   " << description << "\n";
        std::cout << "Humidity:    " << humidity << "%\n";
        std::cout << "Wind Speed:  " << windSpeed << " km/h\n";

        // 3-day forecast
        std::cout << "\n----- 3-Day Forecast -----\n";
        auto weather = data["weather"];
        for (int i = 0; i < 3 && i < weather.size(); i++) {
            std::string date = weather[i]["date"];
            std::string maxTemp = weather[i]["maxtempC"];
            std::string minTemp = weather[i]["mintempC"];
            std::string dayDesc = weather[i]["hourly"][4]["weatherDesc"][0]["value"];

            std::cout << date << ": " << minTemp << "°C ~ " << maxTemp << "°C, " << dayDesc << "\n";
        }
        std::cout << "===========================\n";

    } catch (const std::exception& e) {
        std::cerr << "Error parsing weather data: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string city;

    if (argc > 1) {
        city = argv[1];
    } else {
        std::cout << "Enter city name: ";
        std::getline(std::cin, city);
    }

    if (city.empty()) {
        std::cerr << "Error: City name cannot be empty\n";
        return 1;
    }

    std::cout << "Fetching weather for " << city << "...\n";

    std::string response = fetchWeather(city);

    if (response.find("Error") == 0) {
        std::cerr << response << std::endl;
        return 1;
    }

    displayWeather(city, response);

    return 0;
}
