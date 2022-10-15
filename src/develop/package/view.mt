@:class = import(module:'Matte.Core.Class');


return class(
    name: 'Develop.View',
    define:::(this) {
        @menus = [];


        this.interface = {
            // All the menus within the 
            // view. Should be an array of 
            // arrays, where each inner array 
            // is a string for the menu and a function
            // called when clicking the actions.
            menus :{
                get ::<- empty
            },
            
            
            // called when a view is entered
            onViewActive ::{},
            
            // called when a view is left.
            onViewPause ::{},
            
            x : {get::<-0},
            y : {get::<-1},
            width : {get::<-30},
            height: {get::<-18}
        };
    }
);
