# Lyra UE5 Demo Game
AccelByte Sample game using Unreal Engine's Lyra Demo Game that is already integrated with AccelByte Cloud Service and Blackbox.


## Preparation
Use this command to clone this repo with it's subcommand
```
git clone --recursive https://github.com/AccelByte/ue5-demo-game.git
```

Download the latest version of Unreal Engine 5: https://www.unrealengine.com/en-US/unreal-engine-5


## Building

1. Navigate to `Lyra` folder
2. Right Click and choose `Generate Visual Studio Project Files`
3. LyraStarterGame.sln will be generated on the same folder
4. Open Using Visual Studio
5. Build the game using `DevelopmentEditor` Configuration


## Launch Args
The game has built-it launch argument paramater to perform a certain operation

| Args                             | Description                                                                     |
|----------------------------------|---------------------------------------------------------------------------------|
| -NOSTEAM                         | Disable steam auth                                                              |
| -AUTH_TYPE=ACCELBYTE             | Auto Login using AccelByte Creds, must pass args -AUTH_LOGIN and -AUTH_PASSWORD |
| -AUTH_LOGIN=Username             | AccelByte Cloud Username/Email                                                  |
| -AUTH_PASSWORD=Password          | AccelByte Cloud Password                                                        |
| -CUSTOM_MM_MODE=mm_mode          | Override matchmaking mode                                                       |
|                                  |                                                                                 |
