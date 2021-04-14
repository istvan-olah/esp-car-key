//**************************************************************************************************************************
// COMMON
//**************************************************************************************************************************
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//**************************************************************************************************************************
// PERSISTENT STORAGE
//**************************************************************************************************************************
void configure() {
  if (SerialBT == NULL) {
    SerialBT = new BluetoothSerial();
  }
  SerialBT->begin("Octavia");
  bool started = false;
  bool finished = false;

  String settingsString = "";
  char value;
  while (started == false) {
    value = SerialBT->read();
    if (value == '|') {
      started = true;
      while (finished == false) {
        if (SerialBT->available()) {
          char anotherOne = SerialBT->read();
          if (anotherOne == '|') {
            SerialBT->println("Ok");
            finished = true;
          } else {
            settingsString += anotherOne;
          }
        }
        delay(20);
        checkRestart();
      }
      checkRestart();
    }
    checkRestart();
    if (value == '*') {
      SerialBT->println(getSettings().c_str());
    }

  }
  settings->bleAddress = getValue(settingsString, ';', 0);
  settings->openDelay = getValue(settingsString, ';', 1).toInt();
  settings->openTimeout = getValue(settingsString, ';', 2).toInt();
  settings->closedDelay = getValue(settingsString, ';', 3).toInt();
  settings->closedTimeout = getValue(settingsString, ';', 4).toInt();
  settings->notFoundInARow = getValue(settingsString, ';', 5).toInt();
  settings->onDelay = getValue(settingsString, ';', 6).toInt();

  saveSettings();
  SerialBT->disconnect();
  SerialBT->end();
  delete SerialBT;
}

void checkRestart() {
  if (digitalRead(SETUP_BUTTON) == LOW) {
    delay(1000);
    if (digitalRead(SETUP_BUTTON) == LOW) {
      ESP.restart();
    }
  }
}

void saveSettings() {
  prefs.begin("car");
  prefs.putString("address", settings->bleAddress);
  prefs.putInt("openD", settings->openDelay);
  prefs.putInt("openT", settings->openTimeout);
  prefs.putInt("closedD", settings->closedDelay);
  prefs.putInt("closedT", settings->closedTimeout);
  prefs.putInt("notFound", settings->notFoundInARow);
  prefs.putInt("onDelay", settings->onDelay);
  prefs.end();
}

void loadSettings() {
  prefs.begin("car");
  settings = new Settings();
  settings->bleAddress = prefs.getString("address");
  if (settings->bleAddress == "") {
    settings->bleAddress = "aa:bb:aa:bb:aa:bb";
    settings->openDelay = 3000;
    settings->openTimeout = 5000;
    settings->closedDelay = 1000;
    settings->closedTimeout = 5000;
    settings->notFoundInARow = 3;
    settings->onDelay = 1000;
  }
  else {
    settings->bleAddress = prefs.getString("address");
    settings->openDelay = prefs.getInt("openD");
    settings->openTimeout = prefs.getInt("openT");
    settings->closedDelay = prefs.getInt("closedD");
    settings->closedTimeout = prefs.getInt("closedT");
    settings->notFoundInARow = prefs.getInt("notFound");
    settings->onDelay = prefs.getInt("onDelay");
  }
  prefs.end();
}

String getSettings() {
  String result = String();
  result += "Address: ";
  result += String(settings->bleAddress);
  result += "\nOpen delay: ";
  result += String(settings->openDelay);
  result += "\nOpen timeout: ";
  result += String(settings->openTimeout);
  result += "\nClosed delay: ";
  result += String(settings->closedDelay);
  result += "\nClosed timeout: ";
  result += String(settings->closedTimeout);
  result += "\nNot found in a row: ";
  result += String(settings->notFoundInARow);
  result += "\nOn delay: ";
  result += String(settings->onDelay);
  return result;
}
