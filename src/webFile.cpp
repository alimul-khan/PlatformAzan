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
  <title>Smart-Azan</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>{{CSS}}</style>
  <style>
    /* minimal tab styling */
    .tabs{display:flex;gap:8px;margin:10px 0 14px}
    .tabbtn{flex:1;padding:10px;border:1px solid #ff6f00;border-radius:6px;background:#212121;color:#fff;font-weight:bold;cursor:pointer}
    .tabbtn.active{background:#ff6f00;color:#000}
    .tab{display:none}
    .tab.active{display:block}
  </style>
</head>
<body>
  <div class="container">
    <h2>Azan-Configuration</h2>

    <div class="section">
      <p id="currentTime">Loading...</p>
    </div>

    <div class="tabs">
      <button class="tabbtn active" id="btn-wifi" onclick="showTab('wifi')">Wi-Fi</button>
      <button class="tabbtn" id="btn-loc" onclick="showTab('loc')">Location & Time</button>
    </div>

    <!-- ===== Wi-Fi TAB ===== -->
    <div id="tab-wifi" class="tab active">
      <div class="section">
        <h3>Previous Network Info</h3>
        <p><strong>Previous SSID:</strong> {{SSID}}</p>
        <p><strong>Previous IP:</strong> {{IP}}</p>
      </div>

      <div class="section">
        <h3>Connect with your home Wi-Fi</h3>
        <form action="/data" method="POST">
          <label for="ssid">Select Wi-Fi Network:</label>
          <select id="ssid" name="ssid">
            {{OPTIONS}}
          </select>

          <label for="password">Enter Wi-Fi Password:</label>
          <input type="password" id="password" name="password" required>

          <input type="submit" value="Connect">
        </form>
      </div>
    </div>

    <!-- ===== LOCATION/TIME TAB ===== -->
    <div id="tab-loc" class="tab">
      <div class="section">
        <h3>Update Location & Time Info</h3>
        <form action="/config" method="POST">
          <!-- Latitude -->
          <label for="lat_val">Latitude <span>(N = positive, S = negative)</span>:</label>
          <div>
            <input type="text" id="lat_val" name="lat_val" value="{{LAT_VAL}}">
            <select id="lat_hem" name="lat_hem">
              <option value="N" {{LAT_N_SEL}}>N</option>
              <option value="S" {{LAT_S_SEL}}>S</option>
            </select>
          </div>

          <!-- Longitude -->
          <label for="lon_val">Longitude <span>(E = positive, W = negative)</span>:</label>
          <div>
            <input type="text" id="lon_val" name="lon_val" value="{{LON_VAL}}">
            <select id="lon_hem" name="lon_hem">
              <option value="E" {{LON_E_SEL}}>E</option>
              <option value="W" {{LON_W_SEL}}>W</option>
            </select>
          </div>

          <!-- Time Zone -->
          <label for="tz_val">Time Zone:</label>
          <div>
            <select id="tz_sign" name="tz_sign">
              <option value="+" {{TZ_PLUS_SEL}}>GMT +</option>
              <option value="-" {{TZ_MINUS_SEL}}>GMT -</option>
            </select>
            <input type="text" id="tz_val" name="tz_val" value="{{TZ_VAL}}" placeholder="e.g., 6.5">
          </div>

          <!-- City / Country -->
          <label for="city">City:</label>
          <input type="text" id="city" name="city" value="{{CITY}}">

          <label for="country">Country:</label>
          <input type="text" id="country" name="country" value="{{COUNTRY}}">

          <input type="submit" value="Update Location & Time Info">
        </form>
      </div>
    </div>

  </div>

  <script>
    function updateTime(){
      const now=new Date();
      document.getElementById('currentTime').innerText=now.toLocaleTimeString();
    }
    setInterval(updateTime,1000); updateTime();

    function showTab(which){
      const wifi=document.getElementById('tab-wifi');
      const loc =document.getElementById('tab-loc');
      const bw=document.getElementById('btn-wifi');
      const bl=document.getElementById('btn-loc');

      if(which==='wifi'){
        wifi.classList.add('active'); loc.classList.remove('active');
        bw.classList.add('active');   bl.classList.remove('active');
      }else{
        loc.classList.add('active');  wifi.classList.remove('active');
        bl.classList.add('active');   bw.classList.remove('active');
      }
    }
  </script>
</body>
</html>
)rawliteral";
