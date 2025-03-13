const int PowerPin = D0;
const int RedPin = D4; 
const int BluePin = D5;
const int GreenPin = D6;  

// Capacitive Sensor
int analog_pin = A2;
int tx_pin = D3;

class LedController {

  int PowerPin;
  int RedPin;
  int GreenPin;
  int BluePin;

  public:
    LedController(int powerPin, int redPin, int greenPin, int bluePin) {
      PowerPin = powerPin;
      RedPin = redPin;
      GreenPin = greenPin;
      BluePin = bluePin;
    }

    void setupLed() {
      // Start power
      digitalWrite(PowerPin, HIGH);
      Serial.println("Setup completed!");
      // Turn off all lights
      analogWrite(RedPin, 255); // Turn off red lights
      analogWrite(GreenPin, 255); // Turn off green lights
      analogWrite(BluePin, 255); //Turns off the blue lights
      Serial.println("Reset all lights.");
    }

    void reset() {
      analogWrite(RedPin, 255); // Turns off red
      analogWrite(GreenPin, 255); // Turns off green
      analogWrite(BluePin, 255); // Turns off blue
    }

    void showRed() {
      analogWrite(RedPin, 100);
    }

    void showBlue() {
      analogWrite(RedPin, 100);
      analogWrite(GreenPin, 50);
      analogWrite(BluePin, 0);
    }

    void showGreen() {
      analogWrite(GreenPin, 0);
    }
};

class HighFiveSensor {
  long highScore = 0;
  unsigned long previousMillis = 0;
  const long checkInterval = 1000;

  public:
    HighFiveSensor(long initialHighScore) {
      highScore = initialHighScore;
    }

    bool monitorNewHighScore() {
      int read_high;
      int read_low;
      int diff;
      long int sum = 0;
      int N_samples = 100;    // Number of samples to take.  Larger number slows it down, but reduces scatter.

      for (int i = 0; i < N_samples; i++){
        digitalWrite(tx_pin, HIGH);              // Step the voltage high on conductor 1.
        read_high = analogRead(analog_pin);     // Measure response of conductor 2.
        delayMicroseconds(100);                 // Delay to reach steady state.
        digitalWrite(tx_pin, LOW);               // Step the voltage to zero on conductor 1.
        read_low = analogRead(analog_pin);      // Measure response of conductor 2.
        diff = read_high - read_low;            // desired answer is the difference between high and low.
        sum += diff;                            // Sums up N_samples of these measurements.
      }

      Serial.println(sum);

      if (sum >= highScore) {
        highScore = sum;
        // Make LED light-up!
        return true;
      }

      return false;
    }
};

LedController myLight(PowerPin, RedPin, GreenPin, BluePin);
HighFiveSensor highFiveSensor(0);

int previousMillis = millis();
int interval = 1000;

void setup () {
  Serial.begin(9600);

  pinMode (PowerPin, OUTPUT); 
  pinMode (RedPin, OUTPUT);
  pinMode (GreenPin, OUTPUT);
  pinMode (BluePin, OUTPUT);
  myLight.setupLed();

  // Setup high-five sensor
  pinMode(tx_pin, OUTPUT);
}
  
void loop () {
  bool result = highFiveSensor.monitorNewHighScore();
  if (result) {
    myLight.showBlue();
    previousMillis = millis();
  }

  // Turn off the light after 1 second if it is still on, but don't allow that second to be blocking.
  int currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      myLight.reset();
  }
}