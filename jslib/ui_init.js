function uiInit() {
    var windowWidth, windowHeight; {
        var w = window
            , d = document
            , e = d.documentElement
            , g = d.getElementsByTagName('body')[0];
        windowWidth = w.innerWidth || e.clientWidth || g.clientWidth;
        windowHeight = w.innerHeight || e.clientHeight || g.clientHeight;

    }
    var dimensionLonger = Math.max(windowWidth, windowHeight);
    var dimensionShorter = Math.min(windowWidth, windowHeight);
    var dimensionLongerHalf = dimensionLonger / 2;
    console.log(windowWidth + ' × ' + windowHeight);

    var connectStatusCircle = document.getElementById('connectStatusCircle');
    connectStatusCircle.style.width = connectStatusCircle.style.height = Math.round(dimensionShorter * .05) + 'px';
    document.getElementById('connectStatusText').style.lineHeight = connectStatusCircle.style.height;

    var mouseButtonWidth = Math.round((windowWidth > windowHeight) ? (dimensionLongerHalf * .4) : (dimensionShorter * .4));
    var mouseButtonHeight = Math.round((windowWidth > windowHeight) ? (dimensionShorter * .4) : (dimensionLongerHalf * .8));

    mouseLeftButton.style.width = mouseRightButton.style.width = mouseButtonWidth.toString() + 'px';
    mouseLeftButton.style.height = mouseRightButton.style.height = mouseButtonHeight.toString() + 'px';
    mouseLeftButton.style.lineHeight = mouseRightButton.style.lineHeight = mouseButtonHeight.toString() + 'px';
    mouseLeftButton.style.fontSize = mouseRightButton.style.fontSize = mouseButtonHeight.toString() + 'px';

    if (windowWidth > windowHeight) {
        mouseLeftButton.style.left = '62.5%';
        mouseRightButton.style.left = '87.5%';
        mouseLeftButton.style.top = mouseRightButton.style.top = '50%';
        mouseLeftButton.style.marginTop = mouseRightButton.style.marginTop = '-' + (mouseButtonHeight / 2).toString() + 'px';
        mouseLeftButton.style.marginLeft = mouseRightButton.style.marginLeft = '-' + (mouseButtonWidth / 2).toString() + 'px';
    } else {
        mouseLeftButton.style.left = '25%';
        mouseRightButton.style.left = '75%';
        mouseLeftButton.style.top = mouseRightButton.style.top = '75%';
        mouseLeftButton.style.marginTop = mouseRightButton.style.marginTop = '-' + (mouseButtonHeight / 2).toString() + 'px';
        mouseLeftButton.style.marginLeft = mouseRightButton.style.marginLeft = '-' + (mouseButtonWidth / 2).toString() + 'px';
    }

    var nippleJSoptions = {
        zone: document.getElementById('mouse_joystick')
        , mode: 'static'
        , position: {
            left: ((windowWidth > windowHeight) ? '25%' : '50%')
            , top: ((windowWidth <= windowHeight) ? '25%' : '50%')
        }
        , color: 'red'
        , size: Math.min(dimensionShorter, dimensionLongerHalf) * .8 //set to 25% screen width
    };

    staticJoystick = nipplejs.create(nippleJSoptions);

    staticJoystick[0].ui.front.innerHTML = 'drag';
    staticJoystick[0].ui.front.style.color = 'white';
    staticJoystick[0].ui.front.style.textAlign = 'center';
    staticJoystick[0].ui.front.style.verticalAlign = 'middle';
    staticJoystick[0].ui.front.style.lineHeight = nippleJSoptions.size * .5 + 'px';
    staticJoystick[0].ui.front.style.fontSize = nippleJSoptions.size * .125 + 'px';

}