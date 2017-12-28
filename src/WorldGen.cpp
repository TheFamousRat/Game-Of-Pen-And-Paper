
#include "WorldGen.hpp"

float visualSpace(5.0f);

std::vector<gradientComponent> gradientComponent::standardGradient;
std::vector<gradientComponent> gradientComponent::blueGradient;
std::vector<gradientComponent> gradientComponent::blackWhiteGradient;

void genMap(std::string filename)
{
    WorldMap world(sf::Vector2u(1000,500));

    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(45,45,45),0.0));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(255,255,255),0.148249));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(103,6,190),0.326906));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(0,0,157),0.41436));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(0,177,175),0.547139));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(6,111,0),0.630066));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(255,255,0),0.758334));
    gradientComponent::standardGradient.push_back(gradientComponent(sf::Color(255,0,0),1.0));

    gradientComponent::blueGradient.push_back(gradientComponent(sf::Color(255,255,255),0.0));
    gradientComponent::blueGradient.push_back(gradientComponent(sf::Color(0,0,255),1.0));

    gradientComponent::blackWhiteGradient.push_back(gradientComponent(sf::Color(0,0,0),0.0));
    gradientComponent::blackWhiteGradient.push_back(gradientComponent(sf::Color(255,255,255),1.0));

    sf::Image tempImg;
    sf::Texture tempText;
    sf::Sprite tempSprt;

    sf::RenderWindow window(sf::VideoMode(world.getCanvasWidth(), world.getCanvasHeight()), "Shit");

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

        world.updateMap((float)0.1f);

        window.clear();

        //world.drawTemperatureMap(window);
        world.drawWindsSpeed(window);
        //world.drawDistanceFromSea(window);
        //world.drawOceans(window);

        //world.drawPrecipitation(window);
        //world.drawWinds(window);

        window.display();

        gui.draw();
        guiWindow.display();

    }

    std::cout << "Lhasa : " << world.getPixelAvgTemperaturePeriod(29.6548, 'N', 91.1406, 'E', 0.0f, 12.0f) << "; Real world : 8°C\n";
    std::cout << "Buenos Aires : " << world.getPixelAvgTemperaturePeriod(34.6037, 'S', 58.3816, 'W', 0.0f, 12.0f) << "; Real world : 17.9°C\n";
    std::cout << "Yakutsk : " << world.getPixelAvgTemperaturePeriod(62.0355, 'N', 129.6755, 'E', 0.0f, 12.0f) << "; Real world : -8.8°C\n";
    std::cout << "El Alto : " << world.getPixelAvgTemperaturePeriod(16.5001, 'S', 68.2147, 'W', 0.0f, 12.0f) << "; Real world : 7.2°C\n";

}

WorldMap::WorldMap(sf::Vector2u targetResolution)
{
    maxHeight = 8848.0f;
    minHeight = 0.0f;
    minDepth = 0.0f;
    maxDepth = -10994.0f;
    seaLevel = 0.0f;
    absoluteZero = -273.15;
    lapseRate = -4.0f;
    maxRainShadowDelta = 2000.0f;
    maxRainShadowCoef = 0.9f;

    maxTemperature = 0.0f;
    minTemperature = 0.0f;

    maxSurfacePressure = 0.0f;
    minSurfacePressure = 0.0f;

    pixelCanvasHeight = 0.0f;
    pixelCanvasWidth = 0.0f;

    maxAirDensity = 0.0f;
    minAirDensity = 0.0f;

    minDistanceFromSea = 0.0f;
    maxDistanceFromSea = 0.0f;

    minPrecipitation = 0.0f;
    maxPrecipitation = 0.0f;

    lastMonthNumber = 0.0f;

    if (!createHeightMap(targetResolution))
    {
        std::cerr << "Heightmap failed to load\n";
        exit(-1);
    }

    generatePixelTemperature(lastMonthNumber);
    generatePixelAtmosphericSurfacePressure();
    generatePixelWinds(lastMonthNumber);
    generatePixelPrecipitation();
    generateClimates();

    vertices.resize(pixelCanvasWidth * pixelCanvasHeight);
}

void WorldMap::updateMap(float numberOfMonthsElapsed)
{
    lastMonthNumber += numberOfMonthsElapsed;

    generatePixelTemperature(lastMonthNumber);
    //generatePixelAtmosphericSurfacePressure();
    generatePixelWinds(lastMonthNumber);
    generatePixelPrecipitation(lastMonthNumber);
}

bool WorldMap::createHeightMap(sf::Vector2u targetSize)
{
    sf::Image heightMap;
    sf::Image waterMap;

    if (!heightMap.loadFromFile("C:\\Users\\GUIGUI\\Documents\\Blender\\Inverted earth\\Earth Topography.png")
        || !waterMap.loadFromFile("C:\\Users\\GUIGUI\\Documents\\Blender\\Inverted earth\\landmask4K.png"))
    {
        return false;
    }
    else
    {
        if (targetSize == sf::Vector2u(0,0))
        {
            pixelCanvasWidth = heightMap.getSize().x;
            pixelCanvasHeight = heightMap.getSize().y;
        }
        else
        {
            pixelCanvasWidth = targetSize.x;
            pixelCanvasHeight = targetSize.y;
        }

        resizeImage(waterMap, pixelCanvasWidth, pixelCanvasHeight);
        resizeImage(heightMap, pixelCanvasWidth, pixelCanvasHeight);

        //Create std::vector of the right size
        for (int x(0) ; x < pixelCanvasWidth*pixelCanvasHeight ; x++)
        {
                pixelDataArray.push_back(pixelData());
        }

        //Calculate latitude/longitude/altitude of each pixel
        for (int x(0) ; x < pixelCanvasWidth ; x++)
        {
            for (int y(0) ; y < pixelCanvasHeight ; y++)
            {
                pixelDataArray[x*pixelCanvasHeight+y].altitude = (float)((heightMap.getPixel(x,y).r - seaLevel)/(255.0f - seaLevel))*maxHeight;
                pixelDataArray[x*pixelCanvasHeight+y].latitude = 180.0f*(0.5 - ((double)y/pixelCanvasHeight));

                pixelDataArray[x*pixelCanvasHeight+y].distanceFromSea = -1.0f;

                if (waterMap.getPixel(x,y).r >= 200)
                {
                    pixelDataArray[x*pixelCanvasHeight+y].isWater = false;
                }
                else
                {
                    pixelDataArray[x*pixelCanvasHeight+y].isWater = true;
                    pixelDataArray[x*pixelCanvasHeight+y].oceanDepth = (float)((waterMap.getPixel(x,y).r)/(255.0f))*maxDepth;
                }

            }
        }

        generateDistanceFromSea();
    }

    return true;
}

void WorldMap::generateDistanceFromSea()
{
    //Calculate distance from sea for each pixel                                            //With a pretty shitty algorithm at that
    bool done(true);
    float closestNeighborDistance(0.0f);
    float currentDistance(-1.0f);
    bool leftExists(false);
    bool rightExists(false);
    bool upExists(false);
    bool downExists(false);

    maxDistanceFromSea = pixelDataArray[0].distanceFromSea;
    minDistanceFromSea = pixelDataArray[0].distanceFromSea;

    do
    {
        currentDistance++;
        done = true;

        for (int x(0) ; x < pixelCanvasWidth ; x++)
        {
            for (int y(0) ; y < pixelCanvasHeight ; y++)
            {
                closestNeighborDistance = (10000); std::numeric_limits<float>::max();

                leftExists = (x > 0);
                rightExists = (x < pixelCanvasWidth - 1);
                upExists = (y > 0);
                downExists = (y < pixelCanvasHeight - 1);

                if ((pixelDataArray[x*pixelCanvasHeight+y].distanceFromSea == -1.0f) && !(pixelDataArray[x*pixelCanvasHeight+y].isWater))
                {
                    done = false;

                    if (upExists)
                    {
                        if (leftExists)
                        {
                            if (!( (!pixelDataArray[(x-1)*pixelCanvasHeight+(y-1)].isWater) && pixelDataArray[(x-1)*pixelCanvasHeight+(y-1)].distanceFromSea == -1.0f))
                            {
                                if (closestNeighborDistance > pixelDataArray[(x-1)*pixelCanvasHeight+(y-1)].distanceFromSea + 1.41421356237f)
                                {
                                    closestNeighborDistance = pixelDataArray[(x-1)*pixelCanvasHeight+(y-1)].distanceFromSea + 1.41421356237f;
                                }
                            }
                        }

                        if (rightExists)
                        {
                            if (!( (!pixelDataArray[(x+1)*pixelCanvasHeight+(y-1)].isWater) && pixelDataArray[(x+1)*pixelCanvasHeight+(y-1)].distanceFromSea == -1.0f))
                            {
                                if (closestNeighborDistance > pixelDataArray[(x+1)*pixelCanvasHeight+(y-1)].distanceFromSea + 1.41421356237f)
                                {
                                    closestNeighborDistance = pixelDataArray[(x+1)*pixelCanvasHeight+(y-1)].distanceFromSea + 1.41421356237f;
                                }
                            }
                        }

                        if (!( (!pixelDataArray[x*pixelCanvasHeight+(y-1)].isWater) && pixelDataArray[x*pixelCanvasHeight+(y-1)].distanceFromSea == -1.0f))
                        {
                            if (closestNeighborDistance > pixelDataArray[x*pixelCanvasHeight+(y-1)].distanceFromSea + 1.0f)
                            {
                                closestNeighborDistance = pixelDataArray[x*pixelCanvasHeight+(y-1)].distanceFromSea + 1.0f;
                            }
                        }
                    }

                    if (downExists)
                    {
                        if (leftExists)
                        {
                            if (!( (!pixelDataArray[(x-1)*pixelCanvasHeight+(y+1)].isWater) && pixelDataArray[(x-1)*pixelCanvasHeight+(y+1)].distanceFromSea == -1.0f))
                            {
                                if (closestNeighborDistance > pixelDataArray[(x-1)*pixelCanvasHeight+(y+1)].distanceFromSea + 1.41421356237f)
                                {
                                    closestNeighborDistance = pixelDataArray[(x-1)*pixelCanvasHeight+(y+1)].distanceFromSea + 1.41421356237f;
                                }
                            }
                        }

                        if (rightExists)
                        {
                            if (!( (!pixelDataArray[(x+1)*pixelCanvasHeight+(y+1)].isWater) && pixelDataArray[(x+1)*pixelCanvasHeight+(y+1)].distanceFromSea == -1.0f))
                            {
                                if (closestNeighborDistance > pixelDataArray[(x+1)*pixelCanvasHeight+(y+1)].distanceFromSea + 1.41421356237f)
                                {
                                    closestNeighborDistance = pixelDataArray[(x+1)*pixelCanvasHeight+(y+1)].distanceFromSea + 1.41421356237f;
                                }
                            }
                        }

                        if (!( (!pixelDataArray[x*pixelCanvasHeight+(y+1)].isWater) && pixelDataArray[x*pixelCanvasHeight+(y+1)].distanceFromSea == -1.0f))
                        {
                            if (closestNeighborDistance < pixelDataArray[x*pixelCanvasHeight+(y+1)].distanceFromSea + 1.0f)
                            {
                                closestNeighborDistance = pixelDataArray[x*pixelCanvasHeight+(y+1)].distanceFromSea + 1.0f;
                            }
                        }
                    }

                    if (leftExists)
                    {
                        if (!( (!pixelDataArray[(x-1)*pixelCanvasHeight+y].isWater) && pixelDataArray[(x-1)*pixelCanvasHeight+y].distanceFromSea == -1.0f))
                        {
                            if (closestNeighborDistance > pixelDataArray[(x-1)*pixelCanvasHeight+y].distanceFromSea + 1.0f)
                            {
                                closestNeighborDistance = pixelDataArray[(x-1)*pixelCanvasHeight+y].distanceFromSea + 1.0f;
                            }
                        }
                    }

                    if (rightExists)
                    {
                        if (!( (!pixelDataArray[(x+1)*pixelCanvasHeight+y].isWater) && pixelDataArray[(x+1)*pixelCanvasHeight+y].distanceFromSea == -1.0f))
                        {
                            if (closestNeighborDistance > pixelDataArray[(x+1)*pixelCanvasHeight+y].distanceFromSea + 1.0f)
                            {
                                closestNeighborDistance = pixelDataArray[(x+1)*pixelCanvasHeight+y].distanceFromSea + 1.0f;
                            }
                        }
                    }

                    if (closestNeighborDistance <= currentDistance)
                    {
                        pixelDataArray[x*pixelCanvasHeight+y].distanceFromSea = closestNeighborDistance;

                        minDistanceFromSea = std::min(closestNeighborDistance, minDistanceFromSea);
                        maxDistanceFromSea = std::max(closestNeighborDistance, maxDistanceFromSea);
                    }
                }
            }
        }

    } while (!done);
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

    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            phi =  pixelDataArray[x*pixelCanvasHeight+y].latitude -180 *(0.08*std::sin((3.1415926535897*(monthNumber-4.0f))/6.0f));
            phi2 = phi * phi;

            pixelDataArray[x*pixelCanvasHeight+y].temperatureLatitude = 2.653 * (1e-7) * (phi2*phi2)
                                                                                    + 2.7334*(1e-5) * (phi2*phi)
                                                                                    - 9.615*(1e-3) * (phi2)
                                                                                    - 0.0558 * phi
                                                                                    + 27.1705;

            metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelCanvasHeight+y].altitude);
            pixelDataArray[x*pixelCanvasHeight+y].temperatureElevation = ((lapseRate)/1000)* metersAboveSeaLevel;

            if (!x && !y)
            {
                maxTemperature = pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature();
                minTemperature = pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature();
            }
            else
            {
                maxTemperature = std::max(maxTemperature, pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature());
                minTemperature = std::min(minTemperature, pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature());
            }
        }
    }

    //std::clog << "      T bounds : " << minTemperature << ';' << maxTemperature << '\n';
}

void WorldMap::generatePixelAtmosphericSurfacePressure()
{
    float metersAboveSeaLevel(0.0f);
    double universalGasConstant(8.31447);
    double molarMassDryAir(0.0289644);
    float difference(0.0f);

    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            //Altitude
            metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelCanvasHeight+y].altitude);

            //pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure = 101.29 * std::pow(1 - ((0.0065*pixelDataArray[x*pixelCanvasHeight+y].altitude)/(pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature()+0.0065*pixelDataArray[x*pixelCanvasHeight+y].altitude+273.15)), 5.257);
            pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure = 101.325 * exp2((-9.80665*0.0289644*pixelDataArray[x*pixelCanvasHeight+y].altitude)/(8.3144598*(pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature()+273.15)));

            //pixelDataArray[x*pixelCanvasHeight+y].airDensity = ((pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure * 1000)*molarMassDryAir)/(universalGasConstant*(pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature() + 273.15));
            pixelDataArray[x*pixelCanvasHeight+y].airDensity = ((pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure * 1000)*molarMassDryAir)/(universalGasConstant*(pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature() + 273.15));

            difference = std::abs(101.325 * exp2((-9.80665*0.0289644*pixelDataArray[x*pixelCanvasHeight+y].altitude)/(8.3144598*(15.04+273.15)))
                                   - pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure);

            if (!x && !y)
            {
                minSurfacePressure = pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure;
                maxSurfacePressure = pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure;

                minAirDensity = pixelDataArray[x*pixelCanvasHeight+y].airDensity;
                maxAirDensity = pixelDataArray[x*pixelCanvasHeight+y].airDensity;
            }
            else
            {
                minSurfacePressure = std::min(pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure, minSurfacePressure);
                maxSurfacePressure = std::max(pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure, maxSurfacePressure);

                minAirDensity = std::min(pixelDataArray[x*pixelCanvasHeight+y].airDensity, minAirDensity);
                maxAirDensity = std::max(pixelDataArray[x*pixelCanvasHeight+y].airDensity, maxAirDensity);
            }
        }
    }

    //std::cout << "      SurfacePressure bounds : " << pixelData::minSurfacePressure << ';' << pixelData::maxSurfacePressure << '\n';
    //std::cout << "      Air density bounds : " << pixelData::minAirDensity << ';' << pixelData::maxAirDensity << '\n';
}

void WorldMap::generatePixelWinds(float monthNumber)
{
    double phi(0.0);
    const double equator(0.0f);
    const double subtropicalHighs(30.0f);
    const double subpolarLows(60.0f);
    const double maxF(10.0f);
    const double angularSpeed((6731.0f*1000.0f*2*MY_PI)/(24.0f*60.0f*60.0f));
    int nextCellX(0);
    int nextCellY(0);
    float reductionCoef(0.0f);

    //Note : An altitude of -90 indicates the South Pole, and 90, the North Pole

    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            phi =  pixelDataArray[x*pixelCanvasHeight+y].latitude -180 *(0.08*std::sin((3.1415926535897*(monthNumber-4.0f))/6.0f));

            if (phi > equator + subpolarLows)//In the artic circle, from Fmax at 90 to 0 at 60
            {
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y = ((float)(phi - 60.0f)/(-30.0f)) * -maxF;
            }
            else if (phi <= equator + subpolarLows && phi > equator + subtropicalHighs)//Between Northern Europe and Tripolitania, from 0 at 60 to -Fmax at 30
            {
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y = ((float)(phi - 60.0f)/(-30.0f)) * -maxF;
            }
            else if (phi <= equator + subtropicalHighs && phi > equator)//Between Tripolitania and the Equator, from Fmax at 30 to 0 at the equator
            {
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y = ((float)phi/(-30.0f)) * -maxF;
            }
            else if (phi <= equator && phi > equator-subtropicalHighs)//Between the equator and South Madagascar, from 0 at the equator to -Fmax at -30
            {
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y = ((float)phi/(-30.0f)) * -maxF;
            }
            else if (phi <= equator-subtropicalHighs && phi >= equator-subpolarLows)//Between southern Madagascar and the Antarctic circle, from Fmax to 0
            {
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y = ((float)(phi + 60.0f)/(-30.0f)) * -maxF;
            }
            else if (phi < equator-subpolarLows)//In the antarctic circle, from -Fmax at -90 to 0 at -60
            {
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y = ((float)(phi + 60.0f)/(-30.0f)) * -maxF;
            }

            //NEXT : ADD CORIOLIS FORCE
            pixelDataArray[x*pixelCanvasHeight+y].windDirection.x = -0.002f*angularSpeed*std::sin((2*MY_PI*phi)/180.0f)*pixelDataArray[x*pixelCanvasHeight+y].windDirection.y;

            nextCellX = x;
            nextCellY = y;

            //Add in reduction because of altitude differences
            if (pixelDataArray[x*pixelCanvasHeight+y].windDirection.x > 0)
            {
                nextCellX = (x + 1 + pixelCanvasWidth)%pixelCanvasWidth;
            }
            else if (pixelDataArray[x*pixelCanvasHeight+y].windDirection.x < 0)
            {
                nextCellX = (x - 1 + pixelCanvasWidth)%pixelCanvasWidth;
            }

            if (pixelDataArray[x*pixelCanvasHeight+y].windDirection.y > 0)
            {
                nextCellY = (y + 1 + pixelCanvasHeight)%pixelCanvasHeight;
            }
            else if (pixelDataArray[x*pixelCanvasHeight+y].windDirection.y < 0)
            {
                nextCellY = (y - 1 + pixelCanvasHeight)%pixelCanvasHeight;
            }

            if (pixelDataArray[nextCellX*pixelCanvasHeight+nextCellY].altitude > pixelDataArray[x*pixelCanvasHeight+y].altitude)
            {
                reductionCoef = static_cast<float>(pixelDataArray[nextCellX*pixelCanvasHeight+nextCellY].altitude - pixelDataArray[x*pixelCanvasHeight+y].altitude)/maxRainShadowDelta;
                reductionCoef = std::min(maxRainShadowCoef, std::max(0.0f, reductionCoef));//Putting the coef between 0 and maxRainShadowCoef, 0 being no reduction
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.x *= 1.0f - reductionCoef;
                pixelDataArray[x*pixelCanvasHeight+y].windDirection.y *= 1.0f - reductionCoef;
            }
        }
    }
}

void WorldMap::generatePixelPrecipitation(float monthNumber)
{
    float initialCloudQuantity(1.0f);
    float emptyCloudQuantity(0.0f);
    float phi(0.0f);
    float cloudXPos(0.0f);
    float cloudYPos(0.0f);
    float rainFlow(2.0f);

    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            //If the cell is water with land near it, we add rain
            if (pixelDataArray[x*pixelCanvasHeight+y].isWater)
            {
                phi = pixelDataArray[x*pixelCanvasHeight+y].latitude -180 *(0.08*std::sin((3.1415926535897*(monthNumber-4.0f))/6.0f));
                phi = std::min(80.0f,std::max(-80.0f, phi));
                pixelDataArray[x*pixelCanvasHeight+y].cloudVolume = 8.6844e-6 * phi * phi * phi * phi +
                                                                    -3.154e-4 * phi * phi * phi +
                                                                    -0.119687 * phi * phi +
                                                                    2.6927948 * phi +
                                                                    542.579755;
                //We know begin simulating the cloud
                cloudXPos = x;
                cloudYPos = y;


            }
        }
    }

    //We determine minimum and maximum precipitation the map
    minPrecipitation = pixelDataArray[0].precipitation;
    maxPrecipitation = pixelDataArray[0].precipitation;

    for (unsigned int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (unsigned int y(0) ; y < pixelCanvasHeight ; y++)
        {
            minPrecipitation = std::min(minPrecipitation, pixelDataArray[x*pixelCanvasHeight+y].precipitation);
            maxPrecipitation = std::max(maxPrecipitation, pixelDataArray[x*pixelCanvasHeight+y].precipitation);
        }
    }
}

void WorldMap::generateClimates()
{
    std::vector<float> monthlyTempAvg;
    monthlyTempAvg.resize(12);
    float annualAvg(0.0f);
    float coldestMonth(0.0f);
    float hottestMonth(0.0f);
    float cheat(0.0f);

    for (unsigned int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (unsigned int y(0) ; y < pixelCanvasHeight ; y++)
        {
            for (int i(0) ; i < 12 ; i++)
            {
                monthlyTempAvg[i] = cheat+getPixelAvgTemperaturePeriod(x, y, i, i+1);
            }

            annualAvg = cheat+getPixelAvgTemperaturePeriod(x, y, 0.0f, 12.0f);
            hottestMonth = cheat+(*std::max_element(monthlyTempAvg.begin(), monthlyTempAvg.end()));
            coldestMonth = cheat+(*std::min_element(monthlyTempAvg.begin(), monthlyTempAvg.end()));

            //First letter
            if (coldestMonth > 18)
            {
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter = climateGroup::A;
            }
            else if (coldestMonth >= 0 &&
                     coldestMonth <= 18 &&
                     hottestMonth > 10)
            {
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter = climateGroup::C;
            }
            else if (coldestMonth < 0 &&
                     hottestMonth > 10)
            {
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter = climateGroup::D;
            }
            else if (hottestMonth < 10)
            {
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter = climateGroup::E;
            }
            else
            {
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter = climateGroup::B;
            }

            //Second letter
            if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::B)//S or W
            {
                if (0)
                {

                }
                else if (0)
                {

                }
            }
            else if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::E)//T F or M
            {
                if (hottestMonth <= 10 &&
                    hottestMonth >= 0)
                {
                    pixelDataArray[x*pixelCanvasHeight+y].climateCode.secondLetter = seasonalPrecipitationType::T;
                }
                else if (hottestMonth < 0)
                {
                    pixelDataArray[x*pixelCanvasHeight+y].climateCode.secondLetter = seasonalPrecipitationType::F;
                }
                else if (coldestMonth > -10)
                {
                    pixelDataArray[x*pixelCanvasHeight+y].climateCode.secondLetter = seasonalPrecipitationType::M;
                }
            }
            else if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::A ||
                     pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::C ||
                     pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::D)
            {
                if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::A)//m
                {

                }
                else if (0)
                {

                }
                else if (0)
                {

                }
                else if (0)
                {

                }

            }

            //Third letter
            //If B,C or D, the climate gets a third letter
            if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::B ||
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::C ||
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::D)
            {
                if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::B)
                {
                    if (annualAvg > 18.0f) //h
                    {
                        pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::h;
                    }
                    else //k
                    {
                        pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::k;
                    }
                }
                else if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::C ||
                         pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::D)
                {
                    if (hottestMonth > 22) //a
                    {
                        pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::a;
                    }
                    else if (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter == climateGroup::D &&
                             coldestMonth < -38) //d
                    {
                        pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::d;
                    }
                    else if (hottestMonth <= 22)
                    {
                        if (std::count_if(monthlyTempAvg.begin(), monthlyTempAvg.end(), [](int i){return (i > 10);}) < 4 && coldestMonth > -38)//c
                        {
                            pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::c;
                        }
                        else //b
                        {
                            pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::b;
                        }
                    }
                }
            }
            else //Otherwise, if the climate is A or E, it doesn't get any
            {
                pixelDataArray[x*pixelCanvasHeight+y].climateCode.thirdLetter = heatLevel::no;
            }

        }
    }
}

void WorldMap::saveAltitudeMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawAltitudeMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveTemperatureMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawTemperatureMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveSurfacePressureMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawSurfacePressureMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveAirDensityMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawAirDensityMap(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveWinds(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawWinds(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveDistanceFromSea(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawDistanceFromSea(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveAverageAnnualTemperature(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawAverageAnnualTemperature(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveWindsSpeed(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawWindsSpeed(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::savePrecipitationMap(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawPrecipitation(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveOceans(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawOceans(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::saveClimates(std::string const& filename)
{
    sf::RenderTexture targ;
    targ.create(pixelCanvasWidth, pixelCanvasHeight);
    drawClimates(targ);
    targ.getTexture().copyToImage().saveToFile(filename);
}

void WorldMap::drawAltitudeMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelCanvasHeight+y].altitude, minHeight, maxHeight, gradientComponent::standardGradient));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawTemperatureMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelCanvasHeight+y].getTotalTemperature(), TEMPERATURE_DRAW_MIN, TEMPERATURE_DRAW_MAX, gradientComponent::standardGradient));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawSurfacePressureMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelCanvasHeight+y].atmosphericSurfacePressure, 36, 101.29, gradientComponent::standardGradient));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawAirDensityMap(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelCanvasHeight+y].airDensity, minAirDensity, maxAirDensity, gradientComponent::blueGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawWinds(sf::RenderTarget& target)
{
    unsigned int averagedSquareSize(3);
    std::vector<sf::Vertex> windVectors;
    sf::Vector2f averageDirection;
    unsigned int count_(0);

    for (int x(averagedSquareSize/2) ; x < pixelCanvasWidth ; x+=averagedSquareSize)
    {
        for (int y(averagedSquareSize/2) ; y < pixelCanvasHeight ; y+=averagedSquareSize)
        {
            //We first put the origin of the vector
            windVectors.push_back(sf::Vertex(sf::Vector2f(x, y), sf::Color(0,0,0,0)));

            count_ = 0;
            averageDirection = sf::Vector2f(0,0);
            //We then average the directions
            for (int i(x - (averagedSquareSize/2)) ; i < pixelCanvasWidth && i < (x - (averagedSquareSize/2) + averagedSquareSize) ; i++)
            {
                for (int j(y - (averagedSquareSize/2)) ; j < pixelCanvasHeight && j < (y - (averagedSquareSize/2) + averagedSquareSize) ; j++)
                {
                    count_++;
                    averageDirection += pixelDataArray[i*pixelCanvasHeight+j].windDirection;
                }
            }
            averageDirection.x /= (float)count_;
            averageDirection.y /= (float)count_;

            //And we finally add the direction to the origin
            windVectors.push_back(sf::Vertex(sf::Vector2f(x, y) + averageDirection, sf::Color(0,255,255)));
        }
    }

    target.draw(&windVectors[0], windVectors.size(), sf::Lines);
}

void WorldMap::drawDistanceFromSea(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelCanvasHeight+y].distanceFromSea, minDistanceFromSea, maxDistanceFromSea, gradientComponent::standardGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawAverageAnnualTemperature(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(getPixelAvgTemperaturePeriod(x, y, 0.0f, 12.0f), TEMPERATURE_DRAW_MIN, TEMPERATURE_DRAW_MAX, gradientComponent::standardGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawWindsSpeed(sf::RenderTarget& target)
{
    float windSpeed(0.0f);

    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            windSpeed = std::sqrt((pixelDataArray[x*pixelCanvasHeight+y].windDirection.x*pixelDataArray[x*pixelCanvasHeight+y].windDirection.x)
                        +(pixelDataArray[x*pixelCanvasHeight+y].windDirection.y*pixelDataArray[x*pixelCanvasHeight+y].windDirection.y));

            vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(windSpeed, 0,15, gradientComponent::blackWhiteGradient)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawPrecipitation(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            if (pixelDataArray[x*pixelCanvasHeight+y].precipitation)
                vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), colorFromBoundedGradient(pixelDataArray[x*pixelCanvasHeight+y].precipitation, minPrecipitation, maxPrecipitation, gradientComponent::blackWhiteGradient)));
            else
                vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x, y), sf::Color(0,0,0,0)));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawOceans(sf::RenderTarget& target)
{
    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            vertices[x*pixelCanvasHeight+y] = (sf::Vertex(sf::Vector2f(x,y), ((pixelDataArray[x*pixelCanvasHeight+y].isWater) ? (sf::Color(0,0,255)) : (sf::Color(0,0,0,0)) )));
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

void WorldMap::drawClimates(sf::RenderTarget& target)
{
    std::string climateType("");

    for (int x(0) ; x < pixelCanvasWidth ; x++)
    {
        for (int y(0) ; y < pixelCanvasHeight ; y++)
        {
            if (!pixelDataArray[x*pixelCanvasHeight+y].isWater)
            {
                climateType = pixelDataArray[x*pixelCanvasHeight+y].climateCode.giveKoppenClimateCode();

                if (climateType == "Af")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(60,0,251));
                else if (climateType == "Am")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(47,102,251));
                else if (climateType == "Aw")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(69,161,251));
                else if (climateType == "BWh")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(253,15,0));
                else if (climateType == "BWk")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(254,146,143));
                else if (climateType == "BSh")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(243,160,0));
                else if (climateType == "BSk")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(250,217,93));
                else if (climateType == "Csa")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(246,255,9));
                else if (climateType == "Csb")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(200,203,7));
                else if (climateType == "Cwa")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(137,255,147));
                else if (climateType == "Cwb")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(81,198,96));
                else if (climateType == "Cwc")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(33,146,48));
                else if (climateType == "Cfa")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(189,255,75));
                else if (climateType == "Cfb")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(77,255,53));
                else if (climateType == "Cfc")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(14,197,6));
                else if (climateType == "Dsa")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(255,0,249));
                else if (climateType == "Dsb")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(202,0,193));
                else if (climateType == "Dsc")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(150,39,145));
                else if (climateType == "Dsd")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(140,86,141));
                else if (climateType == "Dwa")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(164,167,255));
                else if (climateType == "Dwb")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(80,110,223));
                else if (climateType == "Dwc")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(76,71,176));
                else if (climateType == "Dwd")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(53,0,135));
                else if (climateType == "Dfa")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(0,250,253));
                else if (climateType == "Dfb")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(68,194,249));
                else if (climateType == "Dfc")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(0,123,124));
                else if (climateType == "Dfd")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(2,68,90));
                else if (climateType == "ET")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(169,172,168));
                else if (climateType == "EF")
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(98,99,97));
                else //No type defined
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(0,0,0));

                switch (pixelDataArray[x*pixelCanvasHeight+y].climateCode.firstLetter)
                {
                case A:
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(60,0,251));
                    break;
                case B:
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(253,15,0));
                    break;
                case C:
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(246,255,9));
                    break;
                case D:
                    vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(255,0,249));
                    break;
                case E:
                    if (climateType == "ET")
                        vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(169,172,168));
                    else if (climateType == "EF")
                        vertices[x*pixelCanvasHeight+y] = sf::Vertex(sf::Vector2f(x,y), sf::Color(98,99,97));
                    break;
                }

            }
        }
    }

    target.draw(&vertices[0], vertices.size(), sf::Points);
}

float WorldMap::getPixelAvgTemperaturePeriod(unsigned int x, unsigned int y, float monthBegin, float monthEnd) const
{
    double l(pixelDataArray[x*pixelCanvasHeight+y].latitude);
    double l2(l*l);
    double l3(l*l*l);
    double l4(l2*l2);

    return ((((lapseRate)/1000)* std::max(0.0f, pixelDataArray[x*pixelCanvasHeight+y].altitude))+
    (((2.653f*(1e-7)*l4*monthEnd - 1.0612f*(1e-6)*l4 + 0.000027334f*l3*monthEnd - 0.000109336f*l3 - 0.00944996f*l2*monthEnd
    - 0.0001576f*l2*std::sin(1.0472f*(monthEnd - 4.0f)) + 0.0377998f*l2 +
    (0.0000291851f*l3 + 0.00225522f*l2 - 0.524324f*l - 1.4177f)*std::cos(0.523599f*(monthEnd - 4.0f))
    - 0.047298f*l*monthEnd - 0.00811878f*l*std::sin(1.0472f*(monthEnd - 4.0f)) + (-0.000504318f*l - 0.01299f)*std::cos(1.5708f*(monthEnd - 4.0f))
    + 0.189192f*l + 26.1779f*monthEnd + 0.946507*std::sin(1.0472*(monthEnd - 4.0f)) + 0.00068083*std::sin(2.0944*(monthEnd - 4.0f)))
    -
    (2.653f*(1e-7)*l4*monthBegin - 1.0612f*(1e-6)*l4 + 0.000027334f*l3*monthBegin - 0.000109336f*l3 - 0.00944996f*l2*monthBegin
    - 0.0001576f*l2*std::sin(1.0472f*(monthBegin - 4.0f)) + 0.0377998f*l2 +
    (0.0000291851f*l3 + 0.00225522f*l2 - 0.524324f*l - 1.4177f)*std::cos(0.523599f*(monthBegin - 4.0f))
    - 0.047298f*l*monthBegin - 0.00811878f*l*std::sin(1.0472f*(monthBegin - 4.0f)) + (-0.000504318f*l - 0.01299f)*std::cos(1.5708f*(monthBegin - 4.0f))
    + 0.189192f*l + 26.1779f*monthBegin + 0.946507*std::sin(1.0472*(monthBegin - 4.0f)) + 0.00068083*std::sin(2.0944*(monthBegin - 4.0f))))/(monthEnd - monthBegin)));


}

float WorldMap::getPixelAvgTemperaturePeriod(float latitude, char latitudeLetter, float longitude, char longitudeLetter, float monthBegin, float monthEnd) const
{
    unsigned int x(0);
    unsigned int y(0);

    if (longitudeLetter == 'W')
        longitude *= -1.0f;

    if (latitudeLetter == 'S')
        latitude *= -1.0f;

    x = ((longitude/360.0f) + 0.5f) * static_cast<float>(pixelCanvasWidth);
    y = ((latitude/(180.f)) - 0.5f) * static_cast<float>(-pixelCanvasHeight);

    return getPixelAvgTemperaturePeriod(x,y,monthBegin,monthEnd);
}

float WorldMap::getPixelTemperatureWithMonth(unsigned int x, unsigned int y, float monthNumber) const
{
    double phi(0.0);
    double phi2(0.0);
    double metersAboveSeaLevel(0.0);
    float temperatureLatitude(0.0f);
    float temperatureElevation(0.0f);

    phi =  pixelDataArray[x*pixelCanvasHeight+y].latitude -180 *(0.08*std::sin((3.1415926535897*(monthNumber-4.0f))/6.0f));
    phi2 = phi * phi;

    temperatureLatitude = 2.653 * (1e-7) * (phi2*phi2)
                        + 2.7334*(1e-5) * (phi2*phi)
                        - 9.615*(1e-3) * (phi2)
                        - 0.0558 * phi
                        + 27.1705;

    metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelCanvasHeight+y].altitude);
    temperatureElevation = ((lapseRate)/1000)* metersAboveSeaLevel;

    return (temperatureElevation + temperatureLatitude);
}

float WorldMap::getPixelTemperatureWithPhi(unsigned int x, unsigned int y, float phi) const
{
    double phi2(0.0);
    double metersAboveSeaLevel(0.0);
    float temperatureLatitude(0.0f);
    float temperatureElevation(0.0f);

    phi2 = phi * phi;

    temperatureLatitude = 2.653 * (1e-7) * (phi2*phi2)
                        + 2.7334*(1e-5) * (phi2*phi)
                        - 9.615*(1e-3) * (phi2)
                        - 0.0558 * phi
                        + 27.1705;

    metersAboveSeaLevel = std::max(0.0f, pixelDataArray[x*pixelCanvasHeight+y].altitude);
    temperatureElevation = ((lapseRate)/1000)* metersAboveSeaLevel;

    return (temperatureElevation + temperatureLatitude);
}

float WorldMap::getPixelMinTemperature(unsigned int x, unsigned int y) const
{
    float infl_1(-2.867943735f);
    float infl_2(103.2197738f);
    float latitude = pixelDataArray[x*pixelCanvasHeight+y].latitude;//Faster to write :')

    if (latitude < infl_1 - 14.4f)
    {
        return getPixelTemperatureWithPhi(x, y, latitude - 14.4f);
    }
    else if (latitude >= infl_1 - 14.4f && latitude < infl_1 + 14.4f)
    {
        return std::min(getPixelTemperatureWithPhi(x, y, latitude - 14.4f), getPixelTemperatureWithPhi(x, y, latitude + 14.4f));
    }
    else if (latitude >= infl_1 + 14.4f && latitude < infl_2 - 14.4f)
    {
        return getPixelTemperatureWithPhi(x, y, latitude + 14.4f);
    }
    else if (latitude >= infl_2 - 14.4f)
    {
        return getPixelTemperatureWithPhi(x, y, infl_2);
    }
}

float WorldMap::getPixelMaxTemperature(unsigned int x, unsigned int y) const
{
    float infl_1(-2.867943735f);
    float infl_2(103.2197738f);
    float latitude = pixelDataArray[x*pixelCanvasHeight+y].latitude;

    if (latitude < infl_1 - 14.4f)
    {
        return getPixelTemperatureWithPhi(x, y, latitude + 14.4f);
    }
    else if (latitude >= infl_1 - 14.4f && latitude < infl_1 + 14.4f)
    {
        return getPixelTemperatureWithPhi(x, y, infl_1);
    }
    else if (latitude >= infl_1 + 14.4f && latitude < infl_2 - 14.4f)
    {
        return getPixelTemperatureWithPhi(x, y, latitude - 14.4f);
    }
    else if (latitude >= infl_2 - 14.4f)
    {
        return std::max(getPixelTemperatureWithPhi(x, y, latitude - 14.4f), getPixelTemperatureWithPhi(x, y, latitude + 14.4f));
    }
}

void initGui(tgui::Gui& gui)
{
    tgui::Theme theme{"themes/Black.txt"};

    auto picture = tgui::Picture::create("xubuntu_bg_aluminium.jpg");
    picture->setSize({"100%", "100%"});
    gui.add(picture);

    auto panel = tgui::ScrollablePanel::create();

    auto radioButton = tgui::RadioButton::create();
        radioButton->setRenderer(theme.getRenderer("RadioButton"));
        radioButton->setPosition(20, 140);
        radioButton->setText("Yep!");
        radioButton->setSize(25, 25);
        panel->add(radioButton);

        radioButton = tgui::RadioButton::create();
        radioButton->setRenderer(theme.getRenderer("RadioButton"));
        radioButton->setPosition(20, 170);
        radioButton->setText("Nope!");
        radioButton->setSize(25, 25);
        panel->add(radioButton);

        radioButton = tgui::RadioButton::create();
        radioButton->setRenderer(theme.getRenderer("RadioButton"));
        radioButton->setPosition(20, 200);
        radioButton->setText("Don't know!");
        radioButton->setSize(25, 25);
        panel->add(radioButton);

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
