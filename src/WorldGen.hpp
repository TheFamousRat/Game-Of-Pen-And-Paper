#ifndef WORLDGEN_HPP_INCLUDED
#define WORLDGEN_HPP_INCLUDED

#include <iostream>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <random>
#include <ctime>
#include <cmath>

#define TEMPERATURE_DRAW_MIN -80
#define TEMPERATURE_DRAW_MAX 30
#define MY_PI 3.1415926535897932384626433832795028841971

/**
Month numbers :
0:December
1:January
2:February
3:Mars
4:April
5:May
6:June
7:July
8:August
9:September
10:October
11:November
12=0:December
etc.
*/

//Used for coloration of maps
struct gradientComponent {
    gradientComponent() {};
    gradientComponent(sf::Color color_, float proportion_) {color = color_; proportion = proportion_;};

    sf::Color color;
    float proportion;

    static std::vector<gradientComponent> standardGradient;
    static std::vector<gradientComponent> blueGradient;
    static std::vector<gradientComponent> blackWhiteGradient;
};

void genMap(std::string filename);
sf::Color colorFromBoundedGradient(float value, float minValue, float maxValue, std::vector<gradientComponent> gradient);
void initGui(tgui::Gui& gui);

enum climateGroup{A,B,C,D,E,defc};
enum seasonalPrecipitationType{S,W,w,s,f,m,T,F,M,defs};
enum heatLevel{a,b,c,d,h,k,no,defh};//No because some climates only have two letters

struct climate{
    climateGroup firstLetter;
    seasonalPrecipitationType secondLetter;
    heatLevel thirdLetter;

    climate() {firstLetter = defc; secondLetter = defs; thirdLetter = defh;};

    std::string giveKoppenClimateCode() const
    {
        std::string str("");

        switch (firstLetter)
        {
            case A:
                str += "A";
                break;
            case B:
                str += "B";
                break;
            case C:
                str += "C";
                break;
            case D:
                str += "D";
                break;
            case E:
                str += "E";
                break;
            case defc:
                str += "0";
                break;
        }

        switch (secondLetter)
        {
            case S:
                str += "S";
                break;
            case W:
                str += "W";
                break;
            case w:
                str += "w";
                break;
            case s:
                str += "s";
                break;
            case f:
                str += "f";
                break;
            case m:
                str += "m";
                break;
            case T:
                str += "T";
                break;
            case F:
                str += "F";
                break;
            case M:
                str += "M";
                break;
            case defs:
                str += "0";
                break;
        }

        switch (thirdLetter)
        {
            case a:
                str += "a";
                break;
            case b:
                str += "b";
                break;
            case c:
                str += "c";
                break;
            case d:
                str += "d";
                break;
            case h:
                str += "h";
                break;
            case k:
                str += "k";
                break;
            case no:
                break;
            case defh:
                str += "0";
                break;
        }
    };
};

struct pixelData{
    float latitude;                         //In °
    float temperatureLatitude;              //In °C
    float temperatureElevation;             //In °C
    float annualTemperatureRange;           //In °C
    float atmosphericSurfacePressure;              //In kPa
    float altitude;                         //In m
    float oceanDepth;                       //In m
    double airDensity;                      //In kg/m^3
    bool isWater;
    unsigned int numberOfparticle;          //On the case
    float averageparticleTemp;              //In °C
    float distanceFromSea;                  //In case
    //float windStrength;
    sf::Vector2f windDirection;
    float precipitation;
    float cloudVolume;
    climate climateCode;
    float phi;

    float getTotalTemperature() const {return temperatureElevation + temperatureLatitude;};
};

class WorldMap{
public:
    //Constructors/Destructor
    WorldMap(sf::Vector2u targetResolution);
    ~WorldMap() {};

    //Update functions
    void updateMap(float numberOfMonthsElapsed);

    //Graphical functions
    void saveAltitudeMap(std::string const& filename);
    void saveTemperatureMap(std::string const& filename);
    void saveSurfacePressureMap(std::string const& filename);
    void saveAirDensityMap(std::string const& filename);
    void saveWinds(std::string const& filename);
    void saveDistanceFromSea(std::string const& filename);
    void saveAverageAnnualTemperature(std::string const& filename);
    void saveWindsSpeed(std::string const& filename);
    void savePrecipitationMap(std::string const& filename);
    void saveOceans(std::string const& filename);
    void saveClimates(std::string const& filename);

    void drawAltitudeMap(sf::RenderTarget& target);
    void drawTemperatureMap(sf::RenderTarget& target);
    void drawSurfacePressureMap(sf::RenderTarget& target);
    void drawAirDensityMap(sf::RenderTarget& target);
    void drawWinds(sf::RenderTarget& target);
    void drawDistanceFromSea(sf::RenderTarget& target);
    void drawAverageAnnualTemperature(sf::RenderTarget& target);
    void drawWindsSpeed(sf::RenderTarget& target);
    void drawPrecipitation(sf::RenderTarget& target);
    void drawOceans(sf::RenderTarget& target);
    void drawClimates(sf::RenderTarget& target);

    float getPixelAvgTemperaturePeriod(unsigned int x, unsigned int y, float monthBegin, float monthEnd) const;
    float getPixelAvgTemperaturePeriod(float latitude, char latitudeLetter, float longitude, char longitudeLetter, float monthBegin, float monthEnd) const;
    float getPixelMonthlyAvg(unsigned int x, unsigned int y, float monthNumber) const;
    float getPixelTemperatureWithMonth(unsigned int x, unsigned int y, float monthNumber) const;
    float getPixelTemperatureWithPhi(unsigned int x, unsigned int y, float phi) const;
    float getPixelMinTemperature(unsigned int x, unsigned int y) const;
    float getPixelMaxTemperature(unsigned int x, unsigned int y) const;

    float getCanvasWidth() {return pixelCanvasWidth;};
    float getCanvasHeight() {return pixelCanvasHeight;};

private:

    //Presented in the order in which they must be used
    bool createHeightMap(sf::Vector2u targetSize = sf::Vector2u(0,0));
    void generatePixelTemperature(float monthNumber = 4.0f);
    void generatePixelAtmosphericSurfacePressure();//Also generated air density
    void generatePixelWinds(float monthNumber = 4.0f);
    void generatePixelPrecipitation(float monthNumber = 4.0f);
    void generateClimates();

    void generateDistanceFromSea();

private:
    std::vector<pixelData> pixelDataArray;
    std::vector<sf::Vertex> vertices;       //Drawing/saving purposes
    float lastMonthNumber;

    float maxHeight;
    float minHeight;
    float minDepth;
    float maxDepth;
    float seaLevel;
    float absoluteZero;
    float lapseRate;
    float maxRainShadowDelta; //Height difference (in meters) between two pixels at which the rain shadow is maximum
    float maxRainShadowCoef;  //By what percentage does a total rain shadow effect slow the wind ?

    float maxTemperature;
    float minTemperature;

    float maxSurfacePressure;
    float minSurfacePressure;

    double maxAirDensity;
    double minAirDensity;

    int pixelCanvasHeight;
    int pixelCanvasWidth;

    float minDistanceFromSea;
    float maxDistanceFromSea;

    float minPrecipitation;
    float maxPrecipitation;
};

inline
double exp2(double x) {
  x = 1.0 + x / 1024;
  x *= x; x *= x; x *= x; x *= x;
  x *= x; x *= x; x *= x; x *= x;
  x *= x; x *= x;
  return x;
}

void resizeImage(sf::Image& target, unsigned int targetWidth, unsigned int targetHeight);


#endif // WORLDGEN_HPP_INCLUDED
