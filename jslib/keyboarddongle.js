class KeyboardDongle {

    constructor() {
        this.device = null;
        this.kbdTypeChar = null;
        this.mouseChar = null;
        this.consumerChar = null;
        this.kbdDongleService = null;
        this.onDisconnected = this.onDisconnected.bind(this);
    }

    request() {
        let options = {
            "filters": [{
                "namePrefix": "KBD"
                , "services": ["a6310000-f090-4a97-8de8-4848ca7238eb"]
      }]
        };
        return navigator.bluetooth.requestDevice(options)
            .then(device => {
                this.device = device;
                this.device.addEventListener('gattserverdisconnected', this.onDisconnected);
            });
    }

    connect() {
        if (!this.device) {
            return Promise.reject('Device is not connected.');
        }
        return this.device.gatt.connect()
            .then(server => server.getPrimaryService("a6310000-f090-4a97-8de8-4848ca7238eb"))
            .then(service => {
                this.kbdDongleService = service;
                return this.kbdDongleService.getCharacteristic("a6310001-f090-4a97-8de8-4848ca7238eb");
            }).then(characteristic => {
                this.kbdTypeChar = characteristic;
                return this.kbdDongleService.getCharacteristic("a6310005-f090-4a97-8de8-4848ca7238eb");
            }).then(characteristic => {
                this.mouseChar = characteristic;
                return this.kbdDongleService.getCharacteristic("a6310006-f090-4a97-8de8-4848ca7238eb");
            }).then(characteristic => {
                this.consumerChar = characteristic;
                return;
            });
    }

    writeTypeCharacter(data) {
        if (this.kbdTypeChar != null) {
            return this.kbdTypeChar.writeValue(data);
        } else {
            return Promise.reject("not connected yet");
        }
    }

    writeMouseCharacter(data) {
        if (this.mouseChar != null) {
            return this.mouseChar.writeValue(data);
        } else {
            return Promise.reject("not connected yet");
        }
    }

    disconnect() {
        if (!this.device) {
            return Promise.reject('Device is not connected.');
        }
        return this.device.gatt.disconnect();
    }

    onDisconnected() {

        this.kbdTypeChar = null;
        this.mouseChar = null;
        this.consumerChar = null;
        this.kbdDongleService = null;

        console.log('Device is disconnected.');
    }
}