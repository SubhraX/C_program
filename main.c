#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <string.h>
#include <json-c/json.h>

#define API_KEY "YOUR_API_KEY"
#define API_ENDPOINT "http://api.weatherapi.com/v1/current.json?key=%s&q=%s"

// Function to handle the HTTP response
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // Concatenate the received data
    strncat(userdata, ptr, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL *curl;
    CURLcode res;

    // Location for weather forecast
    char location[100];
    printf("Enter the location (city, country): ");
    fgets(location, sizeof(location), stdin);
    location[strcspn(location, "\n")] = 0;  // Remove newline character

    // URL for the weather API
    char url[200];
    snprintf(url, sizeof(url), API_ENDPOINT, API_KEY, location);

    // Buffer to store the API response
    char response[4096] = {0};

    // Initialize the curl session
    curl = curl_easy_init();
    if (curl) {
        // Set the URL to retrieve weather data
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

        // Perform the HTTP request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return 1;
        }

        // Cleanup the curl session
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize curl\n");
        return 1;
    }

    // Parse the JSON response
    struct json_object *root;
    root = json_tokener_parse(response);
    if (root == NULL) {
        fprintf(stderr, "Failed to parse JSON response\n");
        return 1;
    }

    // Extract the required information from the JSON
    struct json_object *location_obj, *current_obj, *temp_c_obj, *condition_obj;
    json_object_object_get_ex(root, "location", &location_obj);
    json_object_object_get_ex(root, "current", &current_obj);
    json_object_object_get_ex(current_obj, "temp_c", &temp_c_obj);
    json_object_object_get_ex(current_obj, "condition", &condition_obj);

    const char *city = json_object_get_string(json_object_object_get(location_obj, "name"));
    double temperature = json_object_get_double(temp_c_obj);
    const char *condition = json_object_get_string(json_object_object_get(condition_obj, "text"));

    // Display the weather information
    printf("Weather in %s:\n", city);
    printf("Temperature: %.1f°C\n", temperature);
    printf("Condition: %s\n", condition);

    // Cleanup the JSON object
    json_object_put(root);

    return 0;
}
