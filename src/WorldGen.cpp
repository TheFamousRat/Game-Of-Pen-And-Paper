
#include "WorldGen.hpp"

float pixelData::maxHeight(8848.0f);
float pixelData::minHeight(0.0f);
float pixelData::minDepth(0.0f);
float pixelData::maxDepth(-10994.0f);
float pixelData::seaLevel(0.0f);
float pixelData::absoluteZero(-273.15);
float pixelData::lapseRate(-4.0f);

float pixelData::maxTemperature(0.0f);
float pixelData::minTemperature(0.0f);

float pixelData::maxPressure(0.0f);
float pixelData::minPressure(0.0f);

float pixelData::pixelCanvasHeight(0.0f);
float pixelData::pixelCanvasWidth(0.0f);

double pixelData::maxAirDensity(0.0);
double pixelData::minAirDensity(0.0);

sf::Color particle::caseColor(255,0,0,60);

float visualSpace(5.0f);

std::vector<gradientComponent> gradientComponent::standardGradient;
std::vector<gradientComponent> gradientComponent::blueGradient;
std::vector<gradientComponent> gradientComponent::blackWhiteGradient;

void genMap(std::string filename)
{
    WorldMap world;

    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(45,45,45),0.0));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(255,255,255),0.148249));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(103,6,190),0.326906));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(0,0,157),0.41436));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(0,177,175),0.547139));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(6,111,0),0.630066));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(230,230,0),0.758334));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(255,0,0),1.0));

    gradientComponent::blueGradient.push_back(gradientComponent(sf::Color(255,255,255),0.0));
    gradientComponent::blueGradient.push_back(gradientComponent(sf::Color(0,0,255),1.0));

    gradientComponent::blackWhiteGradient.push_back(gradientComponent(sf::Color(0,0,0),0.0));
    gradientComponent::blackWhiteGradient.push_back(gradientComponent(sf::Color(255,255,255),1.0));

    sf::Image tempImg;
    sf::Texture tempText;
    sf::Sprite tempSprt;

    sf::RenderWindow window(sf::VideoMode(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight), "Shit");

    sf::RenderWindow guiWindow(sf::VideoMode(600, 320), "Parameters");
    tgui::Gui gui(guiWindow);
    initGui(gui);

    sf::Event event;
    sf::Clock timer;
    unsigned int FPS(0);
    timer.restart();

    while (window.isOpen())
    {
        if (timer.getElapsedTime().asSeconds() >= 1.0f)
        {
            std::cout << "FPS : " << FPS << '\n';
            FPS = 0;
            timer.restart();
        }
        else
        {
            FPS++;
        }

        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        while (guiWindow.pollEvent(event))
        {
            gui.handleEvent(event);
        }

        world.updateMap((float)0.15432);

        world.drawTemperatureMap(window);
        //world.drawDistanceFromSea(window);

        window.display();

        gui.draw();
        guiWindow.display();

    }

    std::cout << "Lhasa : " << world.getPixelAvgAnnualTemperature(29.6548, 'N', 91.1406, 'E') << "; Real world : 8°C\n";
    std::cout << "Buenos Aires : " << world.getPixelAvgAnnualTemperature(34.6037, 'S', 58.3816, 'W') << "; Real world : 17.9°C\n";
    std::cout << "Yakutsk : " << world.getPixelAvgAnnualTemperature(62.0355, 'N', 129.6755, 'E') << "; Real world : -8.8°C\n";
    std::cout << "El Alto : " << world.getPixelAvgAnnualTemperature(16.5001, 'S', 68.2147, 'W') << "; Real world : 7.2°C\n";

}

WorldMap::WorldMap()
{
    lastMonthNumber = 0.0f;

    if (!createHeightMap())
    {
        std::cerr << "Heightmap failed to load\n";
        exit(-1);
    }

    generatePixelTemperature(lastMonthNumber);
    generatePixelAtmosphericPressure();

    generateOceanParticles(OCEAN_PARTICLES_QUANTITY);
    generateWindParticles(WIND_PARTICLES_QUANTITY);

    vertices.resize(pixelData::pixelCanvasWidth * pixelData::pixelCanvasHeight);
}

void WorldMap::generateOceanParticles(unsigned int number)
{
    ocean.clear();

    std::default_random_engine gen(time(NULL));
    std::uniform_int_distribution<int> possibleX(0,pixelData::pixelCanvasWidth - 1);
    std::uniform_int_distribution<int> possibleY(0,pixelData::pixelCanvasHeight - 1);

    int tempX(0);
    int tempY(0);

    for (int i(0) ; i < number ; i++)
    {
        do
        {
            tempX = possibleX(gen);
            tempY = possibleY(gen);
        } while (!pixelDataArray[tempX*pixelData::pixelCanvasHeight+tempY].isWater);

        ocean.push_back(particle(sf::Vector2f(tempX, tempY)));
        (*(ocean.rbegin())).temperature = pixelDataArray[tempX*pixelData::pixelCanvasHeight+tempY].temperatureLatitude;
        pixelDataArray[tempX*pixelData::pixelCanvasHeight+tempY].numberOfparticle++;
        //No need to calculate the average temperature, it will just be the current sea-level temperature of the pixel;
        pixelDataArray[tempX*pixelData::pixelCanvasHeight+tempY].averageparticleTemp = pixelDataArray[tempX*pixelData::pixelCanvasHeight+tempY].temperatureLatitude;
    }
}

void WorldMap::generateWindParticles(unsigned int number)
{
    winds.clear();

    std::default_random_engine gen(time(NULL));
    std::uniform_int_distribution<int> possibleX(0,pixelData::pixelCanvasWidth - 1);
    std::uniform_int_distribution<int> possibleY(0,pixelData::pixelCanvasHeight - 1);


    for (int i(0) ; i < number ; i++)
    {
        winds.push_back(particle(sf::Vector2f(possibleX(gen), possibleY(gen))));
    }
}

void WorldMap::updateMap(float numberOfMonthsElapsed)
{
    lastMonthNumber += numberOfMonthsElapsed;

    generatePixelTemperature(lastMonthNumber);
    generatePixelAtmosphericPressure();
    updateOceans(numberOfMonthsElapsed);
    updateWinds(numberOfMonthsElapsed);
}

void WorldMap::updateOceans(float numberOfMonthsElapsed)
{
    float frictionCoefficient(0.5f);//Proportion of the fluid acceleration removed at every iteration

    for (int i(0) ; i < ocean.size() ; i++)
    {
        ocean[i].acceleration *= frictionCoefficient;
    }

    for (int i(0) ; i < ocean.size() ; i++)
    {
        //Calculate all forces applying to the ocean particle


        //Apply Coriolis


        //Project destination pixel


        //Check collisions


        //Move and change temperature
        ocean[i].pos.x += ocean[i].acceleration.x;
        ocean[i].pos.y += ocean[i].acceleration.y;

    }
}

bool WorldMap::createHeightMap(sf::Vector2u targetSize)
{
    sf::Image heightMap;
    sf::Image waterMap;

    if (!heightMap.loadFromFile("C:\\Users\\GUIGUI\\Documents\\Blender\\Inverted earth\\Earth Topography - LowRes.png")
        || !waterMap.loadFromFile("C:\\Users\\GUIGUI\\Documents\\Blender\\Inverted earth\\water_8k.png"))
    {
        return false;
    }
    else
    {
        if (targetSize == sf::Vector2u(0,0))
        {
            pixelData::pixelCanvasWidth = heightMap.getSize().x;
            pixelData::pixelCanvasHeight = heightMap.getSize().y;
        }
        else
        {
            pixelData::pixelCanvasWidth = targetSize.x;
            pixelData::pixelCanvasHeight = targetSize.y;
        }

        resizeImage(waterMap, pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
        resizeImage(heightMap, pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);

        //Create std::vector of the right size
        for (int x(0) ; x < pixelData::pixelCanvasWidth*pixelData::pixelCanvasHeight ; x++)
        {
                pixelDataArray.push_back(pixelData());
        }

        //Calculate latitude/longitude/altitude of each pixel
        for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
        {
            for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
            {
                pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude = (float)((heightMap.getPixel(x,y).r - pixelData::seaLevel)/(255.0f - pixelData::seaLevel))*pixelData::maxHeight;
                pixelDataArray[x*pixelData::pixelCanvasHeight+y].latitude = 180.0f*(0.5 - ((double)y/pixelData::pixelCanvasHeight));

                pixelDataArray[x*pixelData::pixelCanvasHeight+y].distanceFromSea = -1;

                if (waterMap.getPixel(x,y).r <= 40)
                {
                    pixelDataArray[x*pixelData::pixelCanvasHeight+y].isWater = false;
                }
                else
                {
                    pixelDataArray[x*pixelData::pixelCanvasHeight+y].isWater = true;
                    pixelDataArray[x*pixelData::pixelCanvasHeight+y].oceanDepth = (float)((waterMap.getPixel(x,y).r)/(255.0f))*pixelData::maxDepth;
                }

            }
        }

        //Calculate distance from sea for each pixel                                            //With a pretty shitty algorithm at that
        bool done(true);
        decltype(pixelData::distanceFromSea) closestNeighborDistance(0);
        decltype(pixelData::distanceFromSea) currentDistance(-1);
        int count(0);

        do
        {
            currentDistance++;
            done = true;
            count = 0;

            for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
            {
                for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
                {
                    closestNeighborDistance = (10000);

                    if ((pixelDataArray[x*pixelData::pixelCanvasHeight+y].distanceFromSea == -1) && !(pixelDataArray[x*pixelData::pixelCanvasHeight+y].isWater))
                    {
                        done = false;
                        count++;

                        if (x > 0)
                        {
                            if (pixelDataArray[(x-1)*pixelData::pixelCanvasHeight+y].isWater)
                                closestNeighborDistance = -1;
                            else if (pixelDataArray[(x-1)*pixelData::pixelCanvasHeight+y].distanceFromSea != -1 && pixelDataArray[(x-1)*pixelData::pixelCanvasHeight+y].distanceFromSea < closestNeighborDistance)
                                closestNeighborDistance = pixelDataArray[(x-1)*pixelData::pixelCanvasHeight+y].distanceFromSea;
                        }

                        if (x < pixelData::pixelCanvasWidth - 1)
                        {
                            if (pixelDataArray[(x+1)*pixelData::pixelCanvasHeight+y].isWater)
                                closestNeighborDistance = -1;
                            else if (pixelDataArray[(x+1)*pixelData::pixelCanvasHeight+y].distanceFromSea != -1 && pixelDataArray[(x+1)*pixelData::pixelCanvasHeight+y].distanceFromSea < closestNeighborDistance)
                                closestNeighborDistance = pixelDataArray[(x+1)*pixelData::pixelCanvasHeight+y].distanceFromSea;
                        }

                        if (y > 0)
                        {
                            if (pixelDataArray[x*pixelData::pixelCanvasHeight+(y-1)].isWater)
                                closestNeighborDistance = -1;
                            else if (pixelDataArray[x*pixelData::pixelCanvasHeight+(y-1)].distanceFromSea != -1 && pixelDataArray[x*pixelData::pixelCanvasHeight+(y-1)].distanceFromSea < closestNeighborDistance)
                                closestNeighborDistance = pixelDataArray[x*pixelData::pixelCanvasHeight+(y-1)].distanceFromSea;
                        }

                        if (y < pixelData::pixelCanvasHeight - 1)
                        {
                            if (pixelDataArray[x*pixelData::pixelCanvasHeight+(y+1)].isWater)
                                closestNeighborDistance = -1;
                            else if (pixelDataArray[x*pixelData::pixelCanvasHeight+(y+1)].distanceFromSea != -1 && pixelDataArray[x*pixelData::pixelCanvasHeight+(y+1)].distanceFromSea < closestNeighborDistance)
                                closestNeighborDistance = pixelDataArray[x*pixelData::pixelCanvasHeight+(y+1)].distanceFromSea;
                        }

                        if (closestNeighborDistance + 1 == currentDistance)
                            pixelDataArray[x*pixelData::pixelCanvasHeight+y].distanceFromSea = closestNeighborDistance + 1;
                    }
                }
            }
            std::cout << count <<'\n';

        } while (!done);

    }

    return true;
}

sf::Color colorFromBoundedGradient(float value, float minValue, float maxValue, std::vector<gradientComponent> gradient)
{
    sf::Color ret(sf::Color(255,255,255,255));

    double boundedValue((double)(value - minValue)/(maxValue - minValue));

    if (boundedValue > 0 && boundedValue < 1)
    {
        for (int i(0) ; i < gradient.size() ; i++)
        {
            if (boundedValue <= gradient[i].proportion)
            {
                double pos((double)(boundedValue - gradient[std::max(0, i-1)].proportion)/(gradient[i].proportion - gradient[std::max(0, i-1)].proportion));

                ret.r = pos * gradient[i].color.r + (1 - pos) * gradient[std::max(0, i-1)].color.r;
                ret.g = pos * gradient[i].color.g + (1 - pos) * gradient[std::max(0, i-1)].color.g;
                ret.b = pos * gradient[i].color.b + (1 - pos) * gradient[std::max(0, i-1)].color.b;
                break;
            }
        }
    }
    else
    {
        if (boundedValue <= 0.0)
        {
            ret.r = gradient[0].color.r;
            ret.g = gradient[0].color.g;
            ret.b = gradient[0].color.b;
        }
        else if (boundedValue >= 1.0)
        {
            ret.r = gradient[gradient.size() - 1].color.r;
            ret.g = gradient[gradient.size() - 1].color.g;
            ret.b = gradient[gradient.size() - 1].color.b;
        }
    }

    return ret;
}

void WorldMap::generatePixelTemperature(float monthNumber)
{
    double phi(0.0);
    double phi2(0.0);
    double metersAboveSeaLevel(0.0);

    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            phi =  pixelDataArray[x*pixelData::pixelCanvasHeight+y].latitude -180 *(0.08*std::sin((3.1415926535897*(monthNumber-4.0f))/6.0f));
            phi2 = phi * phi;

            pixelDataArray[x*pixelData::pixelCanvasHeight+y].temperatureLatitude = 2.653 * (1e-7) * (phi2*phi2)
                                                                                    + 2.7334*(1e-5) * (phi2*phi)
                                                                                    - 9.615*(1e-3) * (phi2)
                                                                                    - 0.0558 * phi
                                                                                    + 27.1705;

            metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude);
            pixelDataArray[x*pixelData::pixelCanvasHeight+y].temperatureElevation = ((pixelData::lapseRate)/1000)* metersAboveSeaLevel;

            if (!x && !y)
            {
                pixelData::maxTemperature = pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature();
                pixelData::minTemperature = pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature();
            }
            else
            {
                pixelData::maxTemperature = std::max(pixelData::maxTemperature, pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature());
                pixelData::minTemperature = std::min(pixelData::minTemperature, pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature());
            }
        }
    }

    //std::clog << "      T bounds : " << pixelData::minTemperature << ';' << pixelData::maxTemperature << '\n';
}

void WorldMap::generatePixelAtmosphericPressure()
{
    float metersAboveSeaLevel(0.0f);
    double universalGasConstant(8.31447);
    double molarMassDryAir(0.0289644);

    float EARTH_RADIUS(6356.766);
    float altitude_km(0.0f);

    float minDifference(0.0f);
    float maxDifference(0.0f);
    float difference(0.0f);

    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            //Altitude
            metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude);

            //pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure = 101.29 * std::pow(1 - ((0.0065*pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude)/(pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature()+0.0065*pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude+273.15)), 5.257);
            pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure = 101.325 * exp2((-9.80665*0.0289644*pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude)/(8.3144598*(pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature()+273.15)));

            //pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity = ((pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure * 1000)*molarMassDryAir)/(universalGasConstant*(pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature() + 273.15));
            pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity = ((pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure * 1000)*molarMassDryAir)/(universalGasConstant*(pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature() + 273.15));

            difference = std::abs(101.325 * exp2((-9.80665*0.0289644*pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude)/(8.3144598*(15.04+273.15)))
                                   - pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure);

            if (!x && !y)
            {
                pixelData::minPressure = pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure;
                pixelData::maxPressure = pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure;

                pixelData::minAirDensity = pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity;
                pixelData::maxAirDensity = pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity;
            }
            else
            {
                pixelData::minPressure = std::min(pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure, pixelData::minPressure);
                pixelData::maxPressure = std::max(pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure, pixelData::maxPressure);

                pixelData::minAirDensity = std::min(pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity, pixelData::minAirDensity);
                pixelData::maxAirDensity = std::max(pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity, pixelData::maxAirDensity);
            }
        }
    }

    //std::cout << "      Pressure bounds : " << pixelData::minPressure << ';' << pixelData::maxPressure << '\n';
    //std::cout << "      Air density bounds : " << pixelData::minAirDensity << ';' << pixelData::maxAirDensity << '\n';
}

void WorldMap::updateWinds(float numberOfMonthsElapsed)
{
    double inverseNegativeDensity(0.0);


}

void WorldMap::saveAltitudeMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawAltitudeMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveTemperatureMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawTemperatureMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::savePressureMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawPressureMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveAirDensityMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawAirDensityMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveWindGradient(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawWindGradient(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveOcean(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawOcean(targ);
    targ.getTexture().copyToImage().saveToFile(filename); //Yum
}

void WorldMap::saveDistanceFromSea(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelData::pixelCanvasWidth, pixelData::pixelCanvasHeight);
    drawDistanceFromSea(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::drawAltitudeMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            vertices[x*pixelData::pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude, pixelData::minHeight, pixelData::maxHeight, gradientComponent::standardGradient));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawTemperatureMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            //vertices[x*pixelData::pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature(), -80, 30, gradientComponent::standardGradient));
            if (pixelDataArray[x*pixelData::pixelCanvasHeight+y].isWater == false)
                vertices[x*pixelData::pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelData::pixelCanvasHeight+y].getTotalTemperature(), -80, 30, gradientComponent::standardGradient));
            else
                vertices[x*pixelData::pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), sf::Color(0,0,255));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawPressureMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            vertices[x*pixelData::pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelData::pixelCanvasHeight+y].atmosphericPressure, 36, 101.29, gradientComponent::standardGradient));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawAirDensityMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            vertices[x*pixelData::pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelData::pixelCanvasHeight+y].airDensity, pixelData::minAirDensity, pixelData::maxAirDensity, gradientComponent::blueGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawWindGradient(sf::RenderTarget& target)
{

}

void WorldMap::drawOcean(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            vertices[x*pixelData::pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(1.0f * pixelDataArray[x*pixelData::pixelCanvasHeight+y].isWater, 0, 1, gradientComponent::blackWhiteGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawDistanceFromSea(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelData::pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelData::pixelCanvasHeight ; y++)
        {
            vertices[x*pixelData::pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelData::pixelCanvasHeight+y].distanceFromSea, 0, 50, gradientComponent::standardGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

float WorldMap::getPixelAvgAnnualTemperature(float latitude, char latitudeLetter, float longitude, char longitudeLetter) const
{
    unsigned int x(0);
    unsigned int y(0);

    if (longitudeLetter == 'W')
        longitude *= -1.0f;

    if (latitudeLetter == 'S')
        latitude *= -1.0f;

    x = ((longitude/360.0f) + 0.5f) * static_cast<float>(pixelData::pixelCanvasWidth);
    y = ((latitude/(180.f)) - 0.5f) * static_cast<float>(-pixelData::pixelCanvasHeight);

    float minimum(0.0f);
    float maximum(0.0f);

    if (x && x < pixelData::pixelCanvasWidth && y && y < pixelData::pixelCanvasHeight)
    {
        float Tbase(0.0f);
        float Te(0.0f);
        float phi(0.0f);
        float phi2(0.0f);
        float metersAboveSeaLevel(0.0f);
        float sum(0.0f);

        for (float i(0) ; i < 12 ; i++)
        {
            phi =  pixelDataArray[x*pixelData::pixelCanvasHeight+y].latitude -180 *(0.08*std::sin((3.1415926535897*(i-4.0f))/6.0f));
            phi2 = phi * phi;

            Tbase = 2.653 * (1e-7) * (phi2*phi2)
                    + 2.7334*(1e-5) * (phi2*phi)
                    - 9.615*(1e-3) * (phi2)
                    - 0.0558 * phi
                    + 27.1705;

            metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelData::pixelCanvasHeight+y].altitude);
            Te = ((pixelData::lapseRate)/1000)* metersAboveSeaLevel;

            sum += Te + Tbase;

            if (!i)
            {
                minimum = Tbase + Te;
                maximum = minimum;
            }
            else
            {
                minimum = std::min(minimum, Te+Tbase);
                maximum = std::max(maximum, Te+Tbase);
            }
        }

        //std::cout << "Minimum : " << minimum << " and maximum : " << maximum << '\n';

        return (sum/12);
    }
}

void initGui(tgui::Gui& gui)
{
    tgui::Theme theme{"themes/Black.txt"};

    auto picture = tgui::Picture::create("xubuntu_bg_aluminium.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);

    auto panel = tgui::Panel::create();


    auto progressBar = tgui::ProgressBar::create();
        progressBar->setRenderer(theme.getRenderer("ProgressBar"));
        progressBar->setPosition(10, 500);
        progressBar->setSize(200, 20);
        progressBar->setValue(50);
        gui.add(progressBar);

    gui.add(panel);
}

void resizeImage(sf::Image& target, unsigned int targetWidth, unsigned int targetHeight)
{
    sf::Sprite sprt;
    sf::Texture text;
    sf::RenderTexture myText;
    myText.create(targetWidth, targetHeight);

    text.loadFromImage(target);
    sprt.setTexture(text);
    sprt.setScale(sf::Vector2f((float)targetWidth/target.getSize().x, (float)targetHeight/target.getSize().y));
    myText.draw(sprt);
    myText.display();
    target = myText.getTexture().copyToImage();
}
