#include "webFile.h"

// CSS content
const String mainCSS = R"rawliteral(
body { 
    font-family: Arial, sans-serif; 
    background: linear-gradient(to bottom right, #0d47a1, #ff6f00); 
    color: #ffffff; 
    margin: 0; 
    padding: 0; 
}
.container { 
    width: 100%; 
    max-width: 600px; 
    margin: 20px auto; 
    padding: 20px; 
    background-color: rgba(0, 0, 0, 0.8); 
    box-sizing: border-box; 
    box-shadow: 0 0 15px rgba(0, 0, 0, 0.5); 
    border-radius: 10px;
}
h2 { 
    color: #ffcc00; 
    text-align: center; 
    font-size: 1.7em; 
    margin-top: 0; 
}
.section { 
    margin-bottom: 20px; 
    padding: 15px; 
    background-color: rgba(255, 102, 0, 0.2); 
    border: 1px solid #ff6f00; 
    border-radius: 8px; 
    box-sizing: border-box; 
}
.section h3 { 
    color: #ff5722; 
    font-size: 1.3em; 
    margin-top: 0; 
}
#currentTime { 
    font-size: 4em; 
    color: #ffcc00; 
    margin: 0;
    padding: 0;
    text-align: center; 
    line-height: 1.2;
}
label { 
    display: block; 
    margin-bottom: 5px; 
    font-weight: bold; 
    font-size: 1em; 
    color: #ffcc00; 
}
input[type="text"], input[type="password"], select, input[type="submit"] { 
    width: 100%; 
    padding: 10px; 
    margin-bottom: 10px; 
    font-size: 1em; 
    border: 1px solid #ff6f00; 
    border-radius: 4px; 
    box-sizing: border-box; 
}
input[type="text"], input[type="password"], select {
    background-color: #212121; 
    color: #ffffff; 
}
input[type="submit"] { 
    background-color: #ff6f00; 
    color: white; 
    cursor: pointer; 
    font-size: 1em; 
    transition: background-color 0.3s ease;
}
input[type="submit"]:hover { 
    background-color: #ff8f00; 
}
)rawliteral";

// HTML content
const String indexHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ThermoCam-Configuration</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>{{CSS}}</style>
</head>
<body>
  <div class="container">
    <h2>Thermo-Cam</h2>
    <div class="section">
      <p id="currentTime">Loading...</p>
    </div>

    <div class="section">
      <h3>Previous Network Info</h3>
      <p><strong>Previous SSID:</strong> {{SSID}}</p>
      <p><strong>Previous IP:</strong> {{IP}}</p>
    </div>

    <div class="section">
      <h3>Connect with your home Wi-Fi</h3>
      <form action="/data" method="POST">
    <form action="/submit" method="GET">
        <label for="ssid">Select Wi-Fi Network:</label>
        <select id="ssid" name="ssid">
            {{OPTIONS}}
        </select>
        <label for="password">Enter Wi-Fi Password:</label>
        <input type="text" id="password" name="password" required>
        <input type="submit" value="Connect">
    </form>
      </form>
    </div>


      <div class="section">
          <h3>Update Email</h3>
          <form action="/update_email" method="POST">
              <label for="email">Email Address:</label>
              <input type="text" id="email" name="email">
              <input type="submit" value="Update Email">
          </form>
      </div>
      

      
      
      <div class="section">
          <h3>Update Post Interval</h3>
          <form action="/update_post_interval" method="POST">
              <label for="post_interval">Post Interval (ms):</label>
              <input type="text" id="post_interval" name="post_interval" placeholder="10000">
              <input type="submit" value="Update Post Interval">
          </form>
      </div>
      
      <div class="section">
          <h3>Update Endpoint</h3>
          <form action="/update_endpoint" method="POST">
              <label for="endpoint">Endpoint:</label>
              <select id="endpointSelect" name="endpoint" onchange="toggleEndpointField()">
                  <option value="default">Use Default</option>
                  <option value="custom">Custom Endpoint</option>
              </select>
              <input type="text" id="custom_endpoint" name="custom_endpoint" placeholder="www.onlyEditIfU.know/it" style="display:none;">
              <input type="submit" value="Update Endpoint">
          </form>
          <script>
              const defaultEndpoint = "http://thermo-cam.com/receive_data.php";
              function toggleEndpointField() {
                  const endpointField = document.getElementById('custom_endpoint');
                  const selectValue = document.getElementById('endpointSelect').value;
                  if (selectValue === "custom") {
                      endpointField.style.display = "block";
                      endpointField.required = true;
                  } else {
                      endpointField.style.display = "none";
                      endpointField.required = false;
                  }
              }
          </script>
      </div>


          <!-- New Location/Time Section -->
        <div class="section">
        <h3>Update Location & Time Info</h3>
        <form action="/config" method="POST">
            <!-- Latitude -->
            <label for="latitude">Latitude <span>(N = positive, S = negative)</span>:</label>
            <div>
            <input type="text" id="lat_val" name="lat_val" value="{{LAT_VAL}}">
            <select id="lat_hem" name="lat_hem">
                <option value="N" {{LAT_N_SEL}}>N</option>
                <option value="S" {{LAT_S_SEL}}>S</option>
            </select>
            </div>

            <!-- Longitude -->
            <label for="longitude">Longitude <span>(E = positive, W = negative)</span>:</label>
            <div>
            <input type="text" id="lon_val" name="lon_val" value="{{LON_VAL}}">
            <select id="lon_hem" name="lon_hem">
                <option value="E" {{LON_E_SEL}}>E</option>
                <option value="W" {{LON_W_SEL}}>W</option>
            </select>
            </div>

            <!-- Time Zone -->
            <label for="timezone">Time Zone:</label>
            <div>
            <select id="tz_sign" name="tz_sign">
                <option value="+" {{TZ_PLUS_SEL}}>GMT +</option>
                <option value="-" {{TZ_MINUS_SEL}}>GMT -</option>
            </select>
            <input type="text" id="tz_val" name="tz_val" value="{{TZ_VAL}}" placeholder="e.g., 6.5">
            </div>

            <!-- City / Country (unchanged) -->
            <label for="city">City:</label>
            <input type="text" id="city" name="city" value="{{CITY}}">

            <label for="country">Country:</label>
            <input type="text" id="country" name="country" value="{{COUNTRY}}">

            <input type="submit" value="Update Location & Time Info">
        </form>
        </div>


  </div>

  <script>
    function updateTime() {
      const now = new Date();
      const timeString = now.toLocaleTimeString();
      document.getElementById('currentTime').innerText = timeString;
    }

    setInterval(updateTime, 1000);
    updateTime();
  </script>
</body>
</html>
)rawliteral";
