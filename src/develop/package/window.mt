@:SES = import(module:'SES.Core');
@:Fetcher = import(module:'fetcher.mt');
@:Editor = import(module:'editor.mt');
@:Button = import(module:'button.mt');
@:class = import(module:'Matte.Core.Class');


@TILE_BG = Fetcher.Tile.newID();
@TILE_FG = Fetcher.Tile.newID();
@PALETTE_GRAY = Fetcher.Palette.newID();



@BACKGROUND_TOP0 = Fetcher.Background.newID();
@BACKGROUND_TOP1 = Fetcher.Background.newID();


@BACKGROUND_BOTTOM0 = Fetcher.Background.newID();
@BACKGROUND_BOTTOM1 = Fetcher.Background.newID();


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
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_TOP0)+i);
    });
    SES.Background.set(index:BACKGROUND_TOP0, show:true, x:0, y:0, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);

    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_TOP1)+i);
    });
    SES.Background.set(index:BACKGROUND_TOP1, show:true, x:16*8, y:0, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);






    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_BOTTOM0)+i);
    });
    SES.Background.set(index:BACKGROUND_BOTTOM0, show:true, x:0, y:19*8, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);

    // sets tile background
    [0, 16]->for(do:::(i) {
        SES.Tile.copy(from:TILE_BG, to: Fetcher.backgroundIDtoTileID(id:BACKGROUND_BOTTOM1)+i);
    });
    SES.Background.set(index:BACKGROUND_BOTTOM1, show:true, x:16*8, y:19*8, layer:0, effect:SES.Background.EFFECTS.COLOR, palette:PALETTE_GRAY);

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

@:views = [
    Editor.new()
];


selectView(view:views[0]);


return class(
    define:::(this) {
        @dialogButtons = {
            yes: Button.new(),
            no: Button.new()  
        };
        
        dialogButtons.yes.enabled = false;
        dialogButtons.no.enabled = false;
        @dialogBG = Fetcher.Background.newID();
        @:dialogOffset = Fetcher.Sprite.newID();
        Fetcher.Sprite.claimIDs(amount:40);
        
        // populate bg
        @bgOffset = Fetcher.backgroundIDtoTileID(id:dialogBG);
        [0, 16*8]->for(do:::(i) {
            SES.Tile.copy(from:TILE_BG, to:bgOffset+i);
        });
        
        @dialogQuestion = SES.Text.createArea(spriteOffset:dialogOffset, defaultPalette:PALETTE_GRAY);
        dialogQuestion. = {
            editable : false,
            text : '',
            widthChars : 30
        };

        this.interface = {
            question::(text => String, onResponse => Function) {
                Button.allowOnly(
                    which:[dialogButtons.yes, dialogButtons.no]
                );
                SES.Background.set(
                    index:dialogBG, 
                    show:true,
                    palette:PALETTE_GRAY,
                    x: (30*8)/2 - (16*8)/2,
                    y: (20*8)/2 - (8*8)/2
                );
                
                dialogQuestion.text = text;                
                dialogQuestion. = {
                    x: (30 * 8) / 2 - text->length * 3,
                    y: (20 * 8) / 2 - 12,                
                    
                };
                
                


                @:response ::(which) {
                    dialogQuestion.text = ' ';
                    dialogButtons.yes.enabled = false;
                    dialogButtons.no.enabled = false;
                    
                    dialogQuestion.x = -1000;
                    dialogButtons.yes.setup(
                        string: ' ',
                        x: -1000,
                        y: 0
                    );

                    dialogButtons.no.setup(
                        string: ' ',
                        x: -1000,
                        y: 0
                    );                    
                    onResponse(which);
                    Button.allowAgain();
                    
                    SES.Background.set(index:dialogBG, show:false);
                    
                   
                };
                
                dialogButtons.yes.setup(
                    string: ' yes ',
                    x: (30 * 8) / 2 - 15,
                    y: (20 * 8) / 2,
                    onClick : response
                );

                dialogButtons.no.setup(
                    string: ' no  ',
                    x: (30 * 8) / 2 - 15,
                    y: (20 * 8) / 2 + 10,
                    
                    onClick : response
                );

            }
        };
    }

).new();


