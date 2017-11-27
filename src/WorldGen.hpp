#ifndef WORLDGEN_HPP_INCLUDED
#define WORLDGEN_HPP_INCLUDED

#include <iostream>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <random>
#include <ctime>

#define OCEAN_PARTICLES_QUANTITY 10000
#define WIND_PARTICLES_QUANTITY 10000

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

class basicVector{
public:
    void setVector(sf::Vector2f origin_, sf::Vector2f end_) {origin = origin_; end = end_;};

    void setLength(float length)
    {
        if (origin != end)
        {
            sf::Vector2f diff(end - origin);
            diff.x *= length/getLength();
            diff.y *= length/getLength();
            end.x = origin.x + diff.x;
            end.y = origin.y + diff.y;
        }
    };

    float getLength() const
    {
        sf::Vector2f diff(end - origin);
        return std::sqrt(diff.x * diff.x + diff.y * diff.y);
    };

    sf::Vector2f getVector() const {return (end - origin);};

    sf::Vector2f getOrigin() const {return origin;};
    sf::Vector2f getEnd() const {return end;};

    void setOrigin(sf::Vector2f origin_) {origin = origin_;};
    void setEnd(sf::Vector2f end_) {end = end_;};

private:
    sf::Vector2f origin;
    sf::Vector2f end;
};

struct pixelData{
    float latitude;                         //In °
    float temperatureLatitude;              //In °C
    float temperatureElevation;             //In °C
    float annualTemperatureRange;           //In °C
    float atmosphericPressure;              //In kPa
    float altitude;                         //In m
    float oceanDepth;                       //In m
    double airDensity;                      //In kg/m^3
    bool isWater;
    unsigned int numberOfparticle;          //On the case
    float averageparticleTemp;              //In °C
    int distanceFromSea;                    //In case
    basicVector wind;

    static float maxHeight;
    static float minHeight;
    static float minDepth;
    static float maxDepth;
    static float seaLevel;
    static float absoluteZero;
    static float lapseRate;

    static float maxTemperature;
    static float minTemperature;

    static float maxPressure;
    static float minPressure;

    static double maxAirDensity;
    static double minAirDensity;

    static float pixelCanvasHeight;
    static float pixelCanvasWidth;

    float getTotalTemperature() const {return temperatureElevation + temperatureLatitude;};
};

struct particle{
public:
    particle() {};
    particle(sf::Vector2f initPos) {pos = initPos;};
    ~particle() {};

    sf::Vector2f pos;
    float temperature;
    sf::Vector3f acceleration;
    sf::Vector3f speed;

    static sf::Color caseColor;
};

class WorldMap{
public:
    //Constructors/Destructor
    WorldMap();
    ~WorldMap() {};

    //Update functions
    void updateMap(float numberOfMonthsElapsed);
    void generateOceanParticles(unsigned int number);
    void generateWindParticles(unsigned int number);

    //Graphical functions
    void saveAltitudeMap(std::string const& filename);
    void saveTemperatureMap(std::string const& filename);
    void savePressureMap(std::string const& filename);
    void saveAirDensityMap(std::string const& filename);
    void saveWindGradient(std::string const& filename);
    void saveOcean(std::string const& filename);
    void saveDistanceFromSea(std::string const& filename);

    void drawAltitudeMap(sf::RenderTarget& target);
    void drawTemperatureMap(sf::RenderTarget& target);
    void drawPressureMap(sf::RenderTarget& target);
    void drawAirDensityMap(sf::RenderTarget& target);
    void drawWindGradient(sf::RenderTarget& target);
    void drawOcean(sf::RenderTarget& target);
    void drawDistanceFromSea(sf::RenderTarget& target);

    float getPixelAvgAnnualTemperature(float latitude, char latitudeLetter, float longitude, char longitudeLetter) const;

private:

    //Presented in the order in which they must be used
    bool createHeightMap(sf::Vector2u targetSize = sf::Vector2u(0,0));
    void generatePixelTemperature(float monthNumber = 4.0f);
    void generatePixelAtmosphericPressure();//Also generated air density
    void updateWinds(float numberOfMonthsElapsed);
    void updateOceans(float numberOfMonthsElapsed);

private:
    std::vector<pixelData> pixelDataArray;
    std::vector<particle> ocean;
    std::vector<particle> winds;
    std::vector<sf::Vertex> vertices;       //Drawing/saving purposes
    float lastMonthNumber;
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
