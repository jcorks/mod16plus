@:SES = import(module:'SES.Core');
@:class = import(module:'Matte.Core.Class');
@:Fetcher = import(module:'fetcher.mt');

@:PALETTE_TEXT = Fetcher.Palette.newID();

SES.Palette.set(
    index : PALETTE_TEXT,
    colors : [
        [0, 0, 0],
        [0.3, 0.3, 0.3],
        [0.6, 0.6, 0.6],
        [1, 1, 1]
    ]
);


@:buttons = [];

@:inputID = SES.Input.addCallback(
    device: SES.Input.DEVICES.POINTER0,
    callback:::(
        event,
        x,
        y,
        button
    ) {
        @clicked = event == SES.Input.EVENTS.POINTER_BUTTON_DOWN;
        
        buttons->foreach(do:::(k, button) {
            when(!button.enabled) empty;            
            button.update(x, y, clicked);
        });
    }
);


@:TILE_BG_UNHOVERED = Fetcher.Tile.newID();
SES.Tile.set(
    index:TILE_BG_UNHOVERED,
    data: [
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1    
    ]
);

@:TILE_BG_HOVERED = Fetcher.Tile.newID();
SES.Tile.set(
    index:TILE_BG_HOVERED,
    data: [
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2
    ]
);



@disabled = [];

return class(
    statics : {
        allowOnly::(which) {
            disabled = [];
            buttons->foreach(do:::(index, button) {
                when(button.enabled == false) empty;
                disabled->push(value:button);
                button.enabled = false;
            });            
            which->foreach(do:::(index, button) {
                button.enabled = true;
            });
        },
        
        allowAgain::() {
            disabled->foreach(do:::(index, button) {
                button.enabled = true;
            });        
            disabled = [];
        }
    
    },
    define:::(this) {
        buttons->push(value:this);
        
        @_onEnter ::{};
        @_onClick ::{};
        @_onLeave ::{};

        @textOffset = Fetcher.Sprite.newID();        
        @:text = SES.Text.createArea(defaultPalette:PALETTE_TEXT, spriteOffset:textOffset);
        Fetcher.Sprite.claimIDs(amount:15);        
        
        @:bg = Fetcher.Background.newID();
        
        




        @_x = 0; @_y = 0;    
        @widthtiles = 0;
        @height = 8;
        @enabled = true;
        @entered = false;
        
        @:redrawBackground ::{
            @:offset = Fetcher.backgroundIDtoTileID(id:bg);
            if (entered) ::<= {
                [0, widthtiles]->for(do:::(i) {
                    SES.Tile.copy(from:TILE_BG_HOVERED, to:offset+i);
                });
            } else ::<= {
                [0, widthtiles]->for(do:::(i) {
                    SES.Tile.copy(from:TILE_BG_UNHOVERED, to:offset+i);
                });            
            };
        };
        
        
        this.interface = {
            setup ::(
                onEnter,
                onLeave,
                onClick,
                x => Number, 
                y => Number,
                string => String
            ) {
                when(string->length > 15) 
                    error(detail:'Button text only allowed to be 15 characters');
            
                if (onEnter != empty) _onEnter = onEnter => Function;
                if (onLeave != empty) _onLeave = onLeave => Function;
                if (onClick != empty) _onClick = onClick => Function;
                text.text = string;                
                widthtiles = ((text.text->length * 6) / 8)->floor;
                _x = x;
                _y = y;
                
                
                text. = {
                    x: _x,
                    y: _y 
                };
                
                SES.Background.set(
                    index:bg,
                    show:true,
                    x: _x,
                    y: _y,
                    palette: PALETTE_TEXT
                );
                
                redrawBackground();
            },
            
            
            enabled : {
                get ::<- enabled,
                set ::(value) {
                    enabled = value;
                    entered = false;
                    redrawBackground();
                }
            },
            
            
            update ::(x, y, clicked) {
                @in = (x >= _x && x <= _x + widthtiles*8 &&
                       y >= _y && y <= _y + height);
        
                if (entered == false && in == true) ::<= {
                    _onEnter();
                    entered = in;
                    redrawBackground();
                };
                if (entered == true && in == false) ::<= {
                    _onLeave();
                    entered = in;
                    redrawBackground();
                };
                    
                
                if (entered && clicked)
                    _onClick();
            }

            
            
        };
        
        redrawBackground();
    }   
);
