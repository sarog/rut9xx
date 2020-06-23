@ECHO OFF
ECHO.

:: Script variables
SET user=%1
SET pw=%2
SET baseimage=%3
SET version=%4
SET twxport=%5
SET here=%~dp0
SET image=thingworx-%version%-testing-image
SET container=thingworx-%version%-testing

:: Assuming semantic versioning with build (MAJOR.MINOR.PATCH-BUILD),
:: licenses are only changed on MINOR releases, so we only want the MAJOR and
:: minor values.
FOR /F "DELIMS=. TOKENS=1,2" %%i IN (%version%) DO SET myversion=%%i.%%j
xcopy /S /Y %here%\docker\twx-licenses\%myversion%\license.bin %here%\docker\license.bin

:: Point docker to artifactory as the main hub. You need to be on the PTC VPN for this to work.
docker login artifactory.rd2.thingworx.io --username %user% --password %pw%

:: Build the Thingworx image.
docker build --build-arg base=%baseimage% --build-arg version=%version% -t %image% %here%docker

docker ps|FINDSTR %container%
if %errorlevel%==0 (
    :: A container with the same name is already running. Stop it.
    docker stop %container%
)

docker ps -a|FINDSTR %container%
if %errorlevel%==0 (
    :: A container with the same name exists. Remove it.
    docker rm %container%
)

:: Create and run the test container.
:: -d     -> detached.
:: --name -> container name.
:: -p     -> port map.
:: -rm    -> remove container upon exit.
docker run -d --rm -p 8080:8080 -p 4443:4443 -p %twxport%:8443 -p 4444:4444 -p 4445:4445 --name %container% %image%