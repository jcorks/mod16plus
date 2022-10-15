@:View = import(module:'view.mt');
@:SES = import(module:'SES.Core');
@:class = import(module:'Matte.Core.Class');
@:Fetcher = import(module:'fetcher.mt');

@:PALETTE__NORMAL_TEXT = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__NORMAL_TEXT,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 0.9, 0.8]

    ]
);

@:PALETTE__VARIABLE = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__VARIABLE,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 0, 0.5]

    ]
);


@:PALETTE__TYPE = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__TYPE,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 0.5, 1]

    ]
);


@:PALETTE__KEYWORD = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__KEYWORD,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0, 1, 0.5]

    ]
);

@:PALETTE__COMMENT = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__COMMENT,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0.5, 0.5, 0.5]

    ]
);

@:PALETTE__INTROSPECT = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__INTROSPECT,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 0.70, 0.5]

    ]
);


@:PALETTE__SYMBOL = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__SYMBOL,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [0.6, 0.6, 0.6]

    ]
);

@:PALETTE__FUNCTION = Fetcher.Palette.newID();
SES.Palette.set(
    index: PALETTE__FUNCTION,
    colors: [
        [0, 0, 0],
        [0, 0, 0],
        [0, 0, 0],
        [1, 1, 0.5]

    ]
);





@:SPRITE__BEGIN_TITLE = Fetcher.Sprite.newID();
Fetcher.Sprite.claimIDs(amount:30);

@:SPRITE__BEGIN_TEXT = Fetcher.Sprite.newID();
Fetcher.Sprite.claimIDs(amount:30*18);



@:Editor = class(
    inherits : [View],
    define:::(this) {
        

        this.constructor = ::{
            @:searches = [
                {token: 'if', palette: PALETTE__KEYWORD},
                {token: 'else', palette: PALETTE__KEYWORD},
                {token: 'return', palette: PALETTE__KEYWORD},
                {token: 'when', palette: PALETTE__KEYWORD},
                {token: 'match', palette: PALETTE__KEYWORD},
                {token: 'default', palette: PALETTE__KEYWORD},
                {token: 'foreach', palette: PALETTE__KEYWORD},
                {token: 'for', palette: PALETTE__KEYWORD},
                {token: '=>', palette: PALETTE__KEYWORD},
                {token: '<-', palette:PALETTE__KEYWORD},


                {token: ':::', palette:PALETTE__TYPE},
                {token: '::', palette:PALETTE__TYPE},
                {token: ':', palette:PALETTE__TYPE},
                {token: 'Function', palette:PALETTE__TYPE},
                {token: 'Object', palette:PALETTE__TYPE},
                {token: 'Number', palette:PALETTE__TYPE},
                {token: 'Empty', palette:PALETTE__TYPE},
                {token: 'String', palette:PALETTE__TYPE},
                {token: '::<=', palette:PALETTE__TYPE},
                
                {token: '@', palette:PALETTE__VARIABLE},
                {token: 'empty', palette: PALETTE__VARIABLE},

                {token: '->', palette:PALETTE__INTROSPECT},
                {token: 'length', palette:PALETTE__INTROSPECT},
                {token: 'keycount', palette:PALETTE__INTROSPECT},
                {token: 'find', palette:PALETTE__INTROSPECT},
                {token: 'search', palette:PALETTE__INTROSPECT},
                {token: 'parse', palette:PALETTE__INTROSPECT},
                {token: 'random', palette:PALETTE__INTROSPECT},
                {token: 'floor', palette:PALETTE__INTROSPECT},
                {token: 'ceil', palette:PALETTE__INTROSPECT},
                {token: 'round', palette:PALETTE__INTROSPECT},

                {token: '.', palette:PALETTE__SYMBOL},
                {token: '}', palette:PALETTE__SYMBOL},
                {token: '{', palette:PALETTE__SYMBOL},
                {token: '~', palette:PALETTE__SYMBOL},
                {token: '%', palette:PALETTE__SYMBOL},
                {token: '^', palette:PALETTE__SYMBOL},
                {token: '&', palette:PALETTE__SYMBOL},
                {token: '*', palette:PALETTE__SYMBOL},
                {token: '(', palette:PALETTE__SYMBOL},
                {token: ')', palette:PALETTE__SYMBOL},
                {token: '-', palette:PALETTE__SYMBOL},
                {token: '+', palette:PALETTE__SYMBOL},
                {token: '=', palette:PALETTE__SYMBOL},
                {token: '[', palette:PALETTE__SYMBOL},
                {token: ']', palette:PALETTE__SYMBOL},
                {token: "'", palette:PALETTE__SYMBOL},
                {token: '"', palette:PALETTE__SYMBOL},
                {token: '/', palette:PALETTE__SYMBOL},
                {token: '?', palette:PALETTE__SYMBOL},
                {token: '<', palette:PALETTE__SYMBOL},
                {token: '>', palette:PALETTE__SYMBOL},
                {token: ',', palette:PALETTE__SYMBOL},
                {token: '|', palette:PALETTE__SYMBOL}



            ];


            @:applySearches::(string) {
                searches->foreach(do:::(i, search) {
                    @:ind = string->search(key:search.token);
                    when(ind == -1) empty;
                    
                    textarea.setColor(
                        paletteID: search.palette,
                        fromY: textarea.cursorY,
                        toY: textarea.cursorY,
                        fromX: ind,
                        toX: ind+search.token->length-1
                    );
                    
                
                });
            };
            
            @title = SES.Text.createArea(defaultPalette:PALETTE__NORMAL_TEXT, spriteOffset:SPRITE__BEGIN_TITLE);
            title.text = 'main.mt';


            @textarea = SES.Text.createArea(
              spriteOffset: SPRITE__BEGIN_TEXT,
              defaultPalette : PALETTE__NORMAL_TEXT,  
              onChange:: {

                

                @:line = textarea.getLine(index:textarea.cursorY);
                textarea.setColor(
                    paletteID: PALETTE__NORMAL_TEXT,
                    fromY: textarea.cursorY,
                    toY: textarea.cursorY,
                    fromX: 0,
                    toX: line->length-1
                );




                @lastWhitespace = -1;
                [::] {
                    [0, line->length]->for(do:::(i) {
                    
                        
                    
                    
                        match(line->charAt(index:i)) {
                          (' ', '\n', '\t', '\r'): ::<= {
                            @:token = line->substr(from:lastWhitespace+1, to:i-1);
                            lastWhitespace = i;
                          },
                          
                          ('('):::<= {
                            if (i > 0 && match(line->charAt(index:i-1)) {
                                ('-', '+', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', ':', ';', '"', "'", '/', '?', '<', '>', ',', '.', '=', '[', ']', '{', '}', '|'): false,
                                default: true
                            })
                                textarea.setColor(paletteID:PALETTE__FUNCTION, 
                                    fromX: lastWhitespace+1,
                                    toX: i-1,
                                    fromY: textarea.cursorY,
                                    toY: textarea.cursorY
                                );
                            
                          } 
                        };
                        
                        
                        if (i < line->length-1 &&
                            line->substr(from:i, to:i+1) == '//'
                        ) ::<= {
                            textarea.setColor(paletteID:PALETTE__COMMENT, 
                                fromX: i,
                                toX: line->length-1,
                                fromY: textarea.cursorY,
                                toY: textarea.cursorY
                            );
                            send();
                              
                        };

                    });

                    applySearches(string:line);  

                };

            });


            // full GBA screen
            textarea. = {
                widthChars  : this.width,
                heightChars : this.height,    
                x : this.x * 8,
                y : this.y * 8,
                editable    : true,
                scrollable  : true,
            };

            title. = {
                x : (this.width - 10) * 8,
                y : 0,
                heightChars : 1,
                widthChars : 8,
                editable : false,
                scrollable : false
            };


            
            return this;
        };





        
        @:newFile ::{};
        @:removeFile ::{};
        @:switchFile ::{};
        
        this.interface = {
            // All the menus within the 
            // view. Should be an array of 
            // arrays, where each inner array 
            // is a string for the menu and a function
            // called when clicking the actions.
            menus : {
                get :: <- [
                    ['File', 
                        [
                            ['New', newFile],
                            ['Remove', removeFile]
                        ]
                    ],
                    
                    ['Switch', switchFile]
                ]
            },
            
            
            // called when a view is entered
            onViewActive ::{


            
            },
            
            // called when a view is left.
            onViewPause ::{}    
        };
    }
    
);

return Editor;
