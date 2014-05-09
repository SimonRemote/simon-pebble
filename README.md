# PebbleMac
(named change soon...it's not only Pebbles anymore)

## Control your Mac computer through your Pebble.
This application(s) will allow you to control your Mac's various scriptable (AppleScript) features through your Pebble or other smart watch. 

I have set up a server similar to FireBase in which it passes data seamlessly from one device to another. In this case that is data from the Pebble to the Mac and vice versa. 

Some existing solutions already exist to some degree:

- [Pebble Slides](https://github.com/luisivan/pebble-slides)
- [Joseph Schmitt's solution](http://joe.sh/using-pebble-control-mac)
- [libpebble](https://github.com/Hexxeh/libpebble)

However, there are downfalls to each. Pebble Slides solution is limited to controlling a computer's keyboard and forces the user to run a python server and deal with ports. The last two both use libpebble, which is connecting the Pebble directly to the Mac using Bluetooth, which isn't reliable and also is inconvinient for the end user.

## Benefits of my solution
### A few easy steps.

1. Load up the application and click on the menu bar icon. Take note of channel code

    ![](http://i.imgur.com/nUC50gL.png)

2. Go to the Pebble application under PebbleMac's settings page and enter that code. Click Submit

    ![](http://i.imgur.com/zMIUhzi.png)

3. Load the Pebble app and control.

    ![](http://i.imgur.com/J2DIfbx.png)
    ![](http://i.imgur.com/WtKslFR.png)

### Network indifferent

My solution will work over any type of internet connection since both clients connect simultaneously to the server. There is no requirement to open ports or be on the same network. Clients can disconnect and connect as they choose and restore sessions.

### Multiple Applications

The Pebble application being developed currently will be released with support for the following applications

- iTunes
- Spotify
- Rdio
- Keynote
- PowerPoint
- VLC

Adding more is easy.

### Two-Way Communication

The server has the ability to send information in both directions, so when you change a song on the Pebble, the next song will be updated on the screen instantly.

## Future ideas 
### All applications
- Reconnect shortcut
- Choose computer?

### PowerPoint
- Volume control
- Notes fetching/displaying
- Force next/previous slide (skip animation events)
- Progress bar

### Keynote
- Do it
- Volume control
- Notes fetching/displaying
- Force next/previous slide (skip animation events)
- Progress bar

### Music Player
- Do it
- Volume control
- Progress bar

### VLC
- Do it
- Volume control
- Progress bar

## Projects Used

- [WebSocket-Node](https://github.com/Worlize/WebSocket-Node)
- [Popup](https://github.com/shpakovski/Popup)
- [SkipStone](https://github.com/Skipstone/Skipstone) (for reference)


