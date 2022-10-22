@:SES = import(module:'SES.Core');
@:Fetcher = import(module:'fetcher.mt');
@:Editor = import(module:'editor.mt');
@:Button = import(module:'button.mt');
@:class = import(module:'Matte.Core.Class');


@TILE_BG = Fetcher.Tile.newID();
@TILE_FG = Fetcher.Tile.newID();
@PALETTE_GRAY = Fetcher.Palette.newID();



@BACKGROUND_TOP0 = Fetcher.Background.newID();


@BACKGROUND_BOTTOM0 = Fetcher.Background.newID();


SES.Tile.set(
    index: TILE_BG,
    data : [
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



SES.Tile.set(
    index: TILE_FG,
    data : [
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4,
        4, 4, 4, 4, 4, 4, 4, 4
    ]
);

SES.Palette.set(
    index: PALETTE_GRAY,
    colors: [
        [0, 0, 0],
        [0.3, 0.3, 0.3],
        [0.6, 0.6, 0.6],
        [1, 1, 1]
    ]
);



@:menuButtons = [
    Button.new(),
    Button.new(),
    Button.new(),
    Button.new(),
    Button.new()
];

@:setBars::{
    



    // sets tile background
    [0, 32]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_TOP0)+i);
    });
    SES.Background.set(index:BACKGROUND_TOP0, show:true, x:0, y:0, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);






    // sets tile background
    [0, 32]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_BOTTOM0)+i);
    });
    SES.Background.set(index:BACKGROUND_BOTTOM0, show:true, x:0, y:19*8, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);


};


@currentView;
@:selectView::(view){
    if (currentView)
        currentView.onViewPause();
    currentView = view;
    view.onViewActive();
    setBars();


    @menuItemCount = view.menus->keycount;
    @iterX = 0;
    view.menus->foreach(do:::(index, menu) {
        menuButtons[index].setup(
            x: iterX,
            y: 0,
            string: menu[0],
            onClick :: {
                menu[1]();
            }
        );
        iterX += menu[0]->length * 6;
    });
    
    [menuItemCount, 5]->for(do:::(i) {
        menuButtons[i].enabled = false;
        menuButtons[i].setup(x:-100, y:-100, string:"");
    });
};

@:views = [];




return class(
    define:::(this) {
        @dialogButtons = {
            yes: Button.new(),
            no: Button.new(),
            ok: Button.new()
        };
        dialogButtons.yes.enabled = false;
        dialogButtons.no.enabled = false;
        dialogButtons.ok.enabled = false;

        dialogButtons.yes.text = ' yes ';
        dialogButtons.no.text =  ' no  ';
        dialogButtons.ok.text =  ' ok  ';

        dialogButtons.yes.enabled = false;
        dialogButtons.no.enabled = false;
        @dialogBG = Fetcher.Background.newID();
        @:dialogOffset = Fetcher.Sprite.newID();
        Fetcher.Sprite.claimIDs(amount:40);
        
        // populate bg
        @bgOffset = Fetcher.backgroundIDtoTileID(id:dialogBG);
        [0, 32*16]->for(do:::(i) {
            SES.Tile.copy(from:TILE_BG, to:bgOffset+i);
        });
        
        @dialogPrompt = SES.Text.createArea(spriteOffset:dialogOffset, defaultPalette:PALETTE_GRAY);
        dialogPrompt. = {
            editable : false,
            text : '',
            widthChars : 30
        };


        @:prepareDialog ::(buttons, prompt) {
            textBoxes->foreach(do:::(index, box) {
                box.editable = false;
            });
            Button.allowOnly(
                which:[...buttons]->map(to:::(value) <- value[0])
            );

            SES.Background.set(
                index:dialogBG, 
                show:true,
                palette:PALETTE_GRAY,
                x: (30*8)/2 - (32*8)/2,
                y: (20*8)/2 - (16*8)/2
            );
            
            dialogPrompt.text = prompt;                
            dialogPrompt. = {
                x: (30 * 8) / 2 - prompt->length * 3,
                y: (20 * 8) / 2 - 12,                
                
            };
                    
            buttons->foreach(do:::(index, button) {
                button[0].enabled = true;
                button[0].setup(
                    x: (30 * 8) / 2 - 15,
                    y: (20 * 8) / 2 + 10*index,
                    onClick : button[1]
                );
            });

        };
        
        @:putAwayDialog :: {
            textBoxes->foreach(do:::(index, box) {
                box.editable = true;
            });
            

            dialogPrompt.text = ' ';
            dialogButtons->foreach(do:::(i, button) {
                button.enabled = false;
            });
                        
            dialogPrompt.x = -1000;           
            Button.allowAgain();
            
            SES.Background.set(index:dialogBG, show:false);
            
                   
        };



        @:textBoxes = [];


        this.interface = {
            start :: {
                views->push(value:Editor.new());
                selectView(view:views[0]);
            },
            
            createTextBox::(
                defaultPalette,
                onChange
            ) {
                @:spriteOffset = Fetcher.Sprite.newID();
                Fetcher.Sprite.claimIDs(amount:(30*8 / 6) * 20);
                @:out = SES.Text.createArea(
                    defaultPalette,
                    spriteOffset,
                    onChange
                );      
                textBoxes->push(value:out);
                return out; 
            },
        
            question::(text => String, onResponse => Function) {
                
                prepareDialog(
                    buttons: [
                        [dialogButtons.yes, ::{
                            putAwayDialog();
                            onResponse(which:true);
                        }],
                        [dialogButtons.no, ::{
                            putAwayDialog();
                            onResponse(which:false);
                        }],
                    ],
                    prompt: text
                );

            },
            
            alert::(text => String, onOkay) {
                
                prepareDialog(
                    buttons: [
                        [dialogButtons.ok, ::{
                            putAwayDialog();
                            if (onOkay != empty) onOkay();
                        }]
                    ],
                    prompt: text
                );

            },
            
        };
    }

).new();


