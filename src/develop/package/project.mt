@:class = import(module:'Matte.Core.Class');


return class(
    name: 'Project',
    define:::(this) {
        @:directory = ".";
    
        this.interface = {
            sources : [
            
            ],
            
            save :: {
                // make object for json
                // save project.json file
                // package command native function
            }
        }
    }
).new();
