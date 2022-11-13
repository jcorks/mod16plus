@:class = import(module:'Matte.Core.Class');
@:SES = import(module:'SES.Core');



@:ses_native__debug_context_enter = getExternalFunction(name:"ses_native__debug_context_enter");
@:ses_native__debug_context_update = getExternalFunction(name:"ses_native__debug_context_update");
@:ses_native__debug_context_leave = getExternalFunction(name:"ses_native__debug_context_leave");
@:ses_native__debug_context_query = getExternalFunction(name:"ses_native__debug_context_query");
@:ses_native__debug_context_is_done = getExternalFunction(name:"ses_native__debug_context_is_done");
@:ses_native__debug_context_is_allowed = getExternalFunction(name:"ses_native__debug_context_is_allowed");

@inDebugContext = false;



@:Text = class(
    name: 'TextArea',
    define:::(this) {
        @spriteOffset_ = 0;
        @onChange_;
        @defaultPalette_ = 0;


        @TEXT_AREA_HEIGHT = 10;
        @TEXT_AREA_WIDTH = 10;
        
        @LINE_LIMIT = 0;
        @LAYER = 31;
        @GLYPH_WIDTH  = 6;
        @GLYPH_HEIGHT = 8;
        
        
        


        @lines = [''];
        @colors = [[]];
        
        @offsetX = 0;
        @offsetY = 0;
        
        @cursorX = 0;
        @cursorY = 0;


        @scrollX = 0;
        @scrollY = 0;
        
        @lastSpriteCount = 0;







        @:drawString::(string, x, y, offset, colors) {
             
            @spr = offset;
            
            @chX = 0;
            @chY = 0;


            @:drawChar::(px, py, code, color) {
                SES.Sprite.set(
                    index: spr,
                    tile: code,
                    show:true,
                    scaleX:1,
                    scaleY:1,
                    centerX: 0,
                    layer: LAYER,
                    centerY: 0,
                    x: px,
                    y: py,
                    effect: SES.Sprite.EFFECTS.Color,
                    palette: color
                );
                spr += 1;
                
            };
            
            [0, string->length]->for(do:::(i) {
                drawChar(
                    px: chX * GLYPH_WIDTH  + x,
                    py: chY * GLYPH_HEIGHT + y,
                    code: string->charCodeAt(index:i),
                    color: colors[i]
                );        


                chX += 1;
            });
            return spr;
        };



        @:clearCanvas:: {
            [spriteOffset_, lastSpriteCount]->for(do:::(i) {
                SES.Sprite.set(
                    index:i,
                    show:false
                );
            });

        };


        @:MIN ::(a, b) <- if (a < b) a else b;


        @:redrawLines :: {
            @spr = spriteOffset_;
            @i = 0;
            [scrollY, MIN(a:lines->keycount, b:scrollY + TEXT_AREA_HEIGHT)]->for(do:::(index) {
                @:line = lines[index];
                @:colorPath = colors[index];
                when(line->length == 0) ::<= {
                    i+=1;
                };
                
                @scrolledLine = line->substr(from:scrollX, to:MIN(a:line->length-1, b:scrollX+TEXT_AREA_WIDTH));
                @scrolledColor = colorPath->subset(from:scrollX, to:MIN(a:line->length-1, b:scrollX+TEXT_AREA_WIDTH));
                when(scrolledLine == empty || scrolledLine->length == 0) ::<= {
                    i+=1;
                };


                if (scrolledColor != empty)
                    spr = drawString(offset:spr, x:offsetX, y: i*8 + offsetY, string:scrolledLine, colors:scrolledColor);
                i+=1;
            });
            
            [spr, lastSpriteCount]->for(do:::(i) {
                SES.Sprite.set(
                    index:i,
                    show:false
                );
            });                    
            
            // cursor
            if (inputCallbackID != empty) ::<= {
                SES.Sprite.set(
                    index: spr,
                    tile: 0,
                    show:true,
                    scaleX:1,
                    scaleY:1,
                    centerX: 0,
                    layer: 0,
                    centerY: 0,
                    x: (cursorX - scrollX) * GLYPH_WIDTH     + offsetX,
                    y: (cursorY - scrollY) * GLYPH_HEIGHT +1 + offsetY,
                    effect: SES.Sprite.EFFECTS.Color,
                    palette:defaultPalette_
                );                    
                lastSpriteCount = spr+1;
            } else 
                lastSpriteCount = spr;


                
        };


        @:insertText ::(src, at, text) {
            when(at >= src->length-1) src + text;
            when(at == 0) text + src;
               
            return src->substr(from:0, to:at-1) + text + src->substr(from:at, to:src->length-1);
        };
        
        @:insertColors ::(src, at, color) {
            when(at >= src->keycount-1) src->push(value:color);
            src->insert(at, value:color);
        };







        @:movedDown :: {
            if (cursorY >= lines->keycount) ::<= {
                cursorY = lines->keycount-1;
                cursorX = lines[cursorY]->length;
                movedLeft();
                movedRight();
            };
            if (cursorX > lines[cursorY]->length) ::<= {
                cursorX = lines[cursorY]->length;
                movedLeft();
                movedRight();

            };


            if (cursorY - scrollY > TEXT_AREA_HEIGHT-2) ::<= {
                scrollY = cursorY - (TEXT_AREA_HEIGHT-1);
            };


        };

        @:movedLeft :: {
            if (cursorX < 0) ::<= {
                when (cursorY == 0) cursorX = 0;
                cursorY -= 1;
                cursorX = lines[cursorY]->length;
                movedUp();
            }; 

            if (cursorX < scrollX + 2 && scrollX > 0) ::<= {
                scrollX = cursorX - 2;
                if (scrollX < 0) scrollX = 0;
            };

            if (cursorX > scrollX + TEXT_AREA_WIDTH) ::<= {
                scrollX = cursorX - TEXT_AREA_WIDTH;
            };
            



        };

        @:movedUp :: {
            if (cursorY < 0) cursorY = 0;
            if (cursorX > lines[cursorY]->length) ::<= {
                cursorX = lines[cursorY]->length;
                movedLeft();
                movedRight();

            };

            if (cursorY - scrollY < 2 && scrollY > 0) ::<= {
                scrollY = cursorY-2;
                
            };


        };


        @:movedRight :: {
            if (cursorX > lines[cursorY]->length) ::<= {
                cursorY += 1;
                cursorX = 0;
                movedDown();
                movedLeft();
            };
            if (cursorY >= lines->keycount) ::<= {
                cursorY = lines->keycount-1;
                cursorX = lines[cursorY]->length;  
            };

                
            if (cursorX > scrollX + TEXT_AREA_WIDTH) ::<= {
                scrollX = cursorX - TEXT_AREA_WIDTH;
            };

        };


        @inputCallbackID;

        @:keyboardCallback = ::(event, text, key) {

            when(event == SES.Input.EVENTS.KEY_DOWN) ::<= {
                match(key) {
                  (SES.Input.KEYS.TAB):::<= {
                    lines[cursorY] = insertText(src:lines[cursorY], at:cursorX, text:'  ');
                    insertColors(src:colors[cursorY], at:cursorX, color:defaultPalette_);
                    cursorX += 2;
                    movedRight();
                    
                    if (onChange_ != empty) onChange_();
                        
                    redrawLines();
                  },

                  (SES.Input.KEYS.BACKSPACE):::<= {
                  
                    // remove "newline"
                    when (lines[cursorY] == '') ::<={
                        when(lines->keycount == 1) empty;
                        when(cursorY == 0) empty;

                        lines->remove(key:cursorY);
                        colors->remove(key:cursorY);
                        cursorY-=1;
                        movedUp();
                        movedLeft();
                        cursorX = lines[cursorY]->length;
                    };
                    
                    
                    // remove newline + merge previous line
                    when(cursorX == 0) ::<= {
                        when(lines->keycount == 1) empty;
                        when(cursorY == 0) empty;
                        @oldText = lines[cursorY];
                        lines->remove(key:cursorY);
                        @oldColor = colors[cursorY];
                        colors->remove(key:cursorY);
                        cursorY-=1;
                        cursorX = lines[cursorY]->length;
                        lines[cursorY] = lines[cursorY] + oldText;
                        oldColor->foreach(do:::(i, color) {
                            colors[cursorY]->push(value:color);
                        });
                        movedUp();
                        movedLeft();
                    
                    };
                  
                    lines[cursorY] = lines[cursorY]->removeChar(index:cursorX-1);
                    cursorX -=1;


                    movedLeft();
                    if (onChange_ != empty) onChange_();
                  },
                  
                  (SES.Input.KEYS.UP):::<= {
                    cursorY -= 1;
                    movedUp();
                  },
                  
                  (SES.Input.KEYS.DOWN):::<= {
                    cursorY += 1;
                    movedDown();

                  },

                  (SES.Input.KEYS.LEFT):::<= {
                    cursorX -= 1;
                    movedLeft();             
                   },
                  
                  (SES.Input.KEYS.RIGHT):::<= {
                    cursorX += 1;
                    movedRight();

                  },


                  
                  (SES.Input.KEYS.RETURN):::<= {
                    when (LINE_LIMIT > 0 && lines->keycount >= LINE_LIMIT)  empty;
                  
                    // return at end
                    when(cursorX >= lines[cursorY]->length) ::<= {
                        cursorY += 1;
                        cursorX = 0;
                        lines->insert(value:'', at:cursorY);  
                        colors->insert(value:[], at:cursorY);              
                        movedDown();
                        movedLeft();

                    };            
                    
                    // return at start
                    when(cursorX == 0) ::<= {
                        @line = lines[cursorY];
                        lines[cursorY] = '';
                        @color = colors[cursorY];
                        colors[cursorY] = [];
                        cursorY += 1;
                        cursorX = 0;
                        lines->insert(value:line, at:cursorY);
                        colors->insert(value:color, at:cursorY);
                        
                        movedDown();
                        movedLeft();
                        
                        
                    };
                    
                    @portion = lines[cursorY]->substr(from:cursorX, to:lines[cursorY]->length-1);
                    @colorPortion = colors[cursorY]->subset(from:cursorX, to:lines[cursorY]->length-1);

                    lines[cursorY] = lines[cursorY]->substr(from:0, to:cursorX-1);
                    colors[cursorY] = colors[cursorY]->subset(from:0, to:cursorX-1);

                    if (colors[cursorY] == empty)
                        colors[cursorY] = [];

                    cursorY += 1;
                    cursorX = 0;
                    lines->insert(value:portion, at:cursorY);                            
                    colors->insert(value:colorPortion, at:cursorY);

                    movedDown();
                    movedLeft();
                    if (onChange_ != empty) onChange_();

                  }
                  
                };
                redrawLines();
            };

            if (text != empty) ::<= {
                // else, just normal text
                @:line = lines[cursorY];
                @:color = colors[cursorY];
                lines[cursorY] = insertText(src:line, at:cursorX, text);
                [0, text->length]->for(do:::(i) {
                    insertColors(src:color, at:cursorX, color:defaultPalette_);
                });
                cursorX += 1;
                movedRight();

                if (onChange_ != empty) onChange_();
                redrawLines();
            };
        };

        
    
        
        
        @pointerCallbackID;
        @scrollCallbackID;
        @ctrlCallbackID;
        @ctrlMod = false;
        @holdMod = false;
        
        
        

    
        
        this.constructor = ::(spriteOffset, defaultPalette, onChange) {
            onChange_ = onChange;
            if (spriteOffset != empty) spriteOffset_ = spriteOffset => Number;
            if (defaultPalette != empty) defaultPalette_ = defaultPalette => Number;
            
            return this;            
        };
        
    
        this.interface = {
            x : {
                set ::(value => Number) {
                    offsetX = value;
                    redrawLines();
                },
                get ::<- offsetX
            },

            y : {
                set ::(value => Number) {
                    offsetY = value;
                    redrawLines();
                },
                get ::<- offsetY
            },

            scrollX : {
                get ::<- scrollX,
                set ::(value) {
                    scrollX = value;
                    if (scrollX < 0) scrollX = 0;
                    redrawLines();
                    
                }
            },

            scrollY : {
                get ::<- scrollY,
                set ::(value) {
                    when (lines->keycount <= TEXT_AREA_HEIGHT) scrollY = 0;
                    scrollY = value;
                    if (scrollY > lines->keycount - TEXT_AREA_HEIGHT) scrollY = lines->keycount - TEXT_AREA_HEIGHT;
                    if (scrollY < 0) scrollY = 0;
                    redrawLines();
                }
            },
            
            setScroll ::(x, y) {
                scrollX = x;
                if (scrollX < 0) scrollX = 0;
                when (lines->keycount <= TEXT_AREA_HEIGHT) scrollY = 0;
                scrollY = y;
                if (scrollY > lines->keycount - TEXT_AREA_HEIGHT) scrollY = lines->keycount - TEXT_AREA_HEIGHT;
                if (scrollY < 0) scrollY = 0;
                redrawLines();
                
            },
                        
                        
            widthChars : {
                set ::(value) {
                    TEXT_AREA_WIDTH = value;
                }
            },
            
            
            
            width : {
                get ::<- TEXT_AREA_WIDTH * GLYPH_WIDTH                            
            },

            heightChars : {
                set ::(value) {
                    TEXT_AREA_HEIGHT = value;
                }
            },
            
            
            height : {
                get ::<- TEXT_AREA_HEIGHT * GLYPH_HEIGHT                            
            },
            
            editable : {
                set::(value => Boolean) {
                    when(value == true) ::<= {
                        when(inputCallbackID != empty) empty;
                        inputCallbackID = SES.Input.addCallback(
                            device:SES.Input.DEVICES.KEYBOARD,
                            callback:keyboardCallback
                        );            
                        pointerCallbackID = SES.Input.addCallback(
                            device:SES.Input.DEVICES.POINTER0,
                            callback:::(event, x, y, button) {
                                if (event == SES.Input.EVENTS.POINTER_BUTTON_DOWN) ::<= {
                                    @:a = this.pixelCoordsToCursor(x, y);        
                                    this.moveCursor(x:a.x, y:a.y);
                                };
                            }
                        );
                        redrawLines();
                    };
                    
                    when(inputCallbackID == empty) empty;
                    SES.Input.removeCallback(id:inputCallbackID, device:SES.Input.DEVICES.KEYBOARD);
                    SES.Input.removeCallback(id:pointerCallbackID, device:SES.Input.DEVICES.POINTER0);
                    inputCallbackID = empty;
                    redrawLines();
                },
                
            },
            
            scrollable : {
                set::(value => Boolean) {
                    when(value == true) ::<= {
                        when(scrollCallbackID != empty) empty;
                        
                        @isDown = false;
                        
                        @lastX = 0;
                        @lastY = 0;
                        @ripple = false;
                        scrollCallbackID = SES.Input.addCallback(
                            device:SES.Input.DEVICES.POINTER0,
                            callback:::(event, x, y, button) {
                                if (event == SES.Input.EVENTS.POINTER_BUTTON_DOWN) holdMod = true;
                                if (event == SES.Input.EVENTS.POINTER_BUTTON_UP  ) holdMod = false;

                                if (event == SES.Input.EVENTS.POINTER_MOTION) ::<= {
                                    if (holdMod && ctrlMod) ::<= {
                                        if (ripple) ::<= {
                                            this.setScroll(
                                                x: this.scrollX - (x - lastX)/2,
                                                y: this.scrollY - (y - lastY)/2
                                            );
                                        };
                                        ripple = !ripple;
                                    };
                                    
                                    lastX = x;
                                    lastY = y;
                                };

                                if (event == SES.Input.EVENTS.POINTER_SCROLL) ::<= {
                                    this.scrollX -= x;
                                    this.scrollY -= y;                                                
                                };    
                                
                            }
                        );
                        
                        ctrlCallbackID = SES.Input.addCallback(
                            device:SES.Input.DEVICES.KEYBOARD,
                            callback:::(event, text, key) {
                                when(event == SES.Input.EVENTS.KEY_DOWN && key == SES.Input.KEYS.LCTRL) ctrlMod = true;
                                when(event == SES.Input.EVENTS.KEY_UP   && key == SES.Input.KEYS.LCTRL) ctrlMod = false;
                            }
                        );
                    };

                    when(scrollCallbackID == empty) empty;
                    SES.Input.removeCallback(id:scrollCallbackID, device:SES.Input.DEVICES.POINTER0);
                    SES.Input.removeCallback(id:ctrlCallbackID, device:SES.Input.DEVICES.KEYBOARD);
                    scrollCallbackID = empty;
                    

                }
            },
            
            text : {
                get :: {
                    @linesReal = [];
                    lines->foreach(do:::(i, line) {
                        if (i != 0)
                            linesReal->push(value:'\n');
                        linesReal->push(value:line);
                        
                    });
                    return String.combine(strings:linesReal);
                },
                
                
                set ::(value)  {
                    cursorX = 1;
                    cursorY = 0;

                    if (value == '' || value == '\n') ::<= {
                        lines = [''];
                        colors = [[]];
                    } else ::<= {                                                                       
                        lines = value->split(token:'\n');
                        [0, lines->keycount]->for(do:::(i) {
                            @:color = [];;
                            @:line = lines[i];
                            [0, line->length]->for(do:::(n) {
                                color->push(value:defaultPalette_);
                            });
                            
                            colors[i] = color;
                        });
                    };

                    if (LINE_LIMIT > 0) ::<= {
                        lines = lines->subset(from:0, to:LINE_LIMIT);
                        colors = colors->subset(from:0, to:LINE_LIMIT);
                        if (lines == empty) ::<= {
                            lines = [''];
                            colors = [[]];
                        };
                    };

                    if (onChange_ != empty) onChange_();

                    redrawLines();  
                },
            },
            
            addLine ::(text) {
                lines->push(value:text);
                @:colorLine = [];
                [0, text->length]->for(do:::(i) {
                    colorLine->push(value:defaultPalette_);
                });
                colors->push(value:colorLine);
                
                if (LINE_LIMIT > 0)
                    lines = lines->subset(from:0, to:LINE_LIMIT);
                                                    
                redrawLines();  
            },
            
            getLine ::(index => Number) => String {
                return lines[index];
            },
            
            getLineCount:: {
                return lines->keycount;
            },
            
            lineLimit : {
                set ::(value) <- LINE_LIMIT = value
            },
            
            setScrollBottom :: {
                if (lines->keycount > TEXT_AREA_HEIGHT) ::<= {
                    scrollY = lines->keycount - TEXT_AREA_HEIGHT;
                };
                redrawLines();
            },
            
            
            // returns an x y set of cursor a cursor position 
            // corresponding to the x y pixel given
            pixelCoordsToCursor::(x, y) {
                @ycursor = scrollY + ((y - offsetY) / GLYPH_HEIGHT);
                if (ycursor < 0) ycursor = 0;
                if (ycursor > lines->keycount-1) ycursor = lines->keycount-1;
                
                ycursor = ycursor->floor;
                
                @xcursor = scrollX + ((x - offsetX) / GLYPH_WIDTH);
                if (xcursor < 0) xcursor = 0;
                if (xcursor > lines[ycursor]->length) xcursor = lines[ycursor]->length;
                
                return {x:xcursor->floor, y:ycursor};
            },
            
            
            // Gets a screen pixel corresponding to the 
            // current cursor location
            cursorToPixelCoords :: {
                @y = offsetY + (cursorY - scrollY) * GLYPH_HEIGHT;                                
                if (y < 0) y = 0;

                @x = offsetX + (cursorX - scrollX) * GLYPH_WIDTH;
                if (x < 0) x = 0;
                
                return {x:x, y:y};
            },
            
            
            moveCursor ::(x, y) {
                cursorX = x;
                cursorY = y;
                redrawLines();
            },
            
            cursorY : {
                get :: <- cursorY
            },

            cursorX : {
                get :: <- cursorX
            },
            
            
            
            setColor ::(paletteID => Number, fromX => Number, fromY => Number, toX => Number, toY => Number) {
                when(toY < fromY) print(message:'toY'+toY+' fromY'+fromY);
                when(toY >= colors->keycount || fromY >= colors->keycount) print(message:'toY'+toY+' numColLines'+colors->keycount);
                when(toY < 0) empty;
                when(fromY == toY) ::<= {
                    @color = colors[fromY];
                    [fromX, toX+1]->for(do:::(x) {
                        color[x] = paletteID;
                    });
                };
                
                
                when(fromY < toY) ::<= {
                    @color = colors[fromY];
                    [fromX, color->length]->for(do:::(x) {
                        color[x] = paletteID;
                    });


                    [fromY, toY]->for(do:::(y) {
                        color = colors[y];
                        [0, color->length]->for(do:::(x) {
                            color[x] = paletteID;
                        });
                    });

                    color = colors[toY];
                    [0, toX+1]->for(do:::(x) {
                        color[x] = paletteID;
                    });

                };
            
            }

        };
    }
);



// activates the console, suspending computation.
//
@:debug ::{    
    when(!ses_native__debug_context_is_allowed()) empty;
   
    when (inDebugContext) empty;
    inDebugContext = true;  
    @display;
    @cursor;
    @entry;
    @callbackID;
    
    



    
    @:onDebugPrint::(text => String, colorHint) {
        display.addLine(text);
        display.setColor(paletteID:colorHint, fromX:0, toX:text->length-1, fromY:display.getLineCount()-1, toY:display.getLineCount()-1);
        display.setScrollBottom();
    };
    
    @:onDebugClear:: {
        display.text = '';
    };
    
    @:onDebugInit :: {

    
        display = Text.new();
        display. = {
            widthChars: (SES.resolutionWidth / 6)->floor,
            heightChars:((SES.resolutionHeight-3) / 8)->floor,
            editable : false,
            scrollable : true
        };


        cursor = Text.new(spriteOffset:3999);
        cursor. = {
            text : '>',
            heightChars: 1,
            editable : false,
            x: 0,
            y: SES.resolutionHeight - 8,            
            widthChars: 1,
        };         

        entry = Text.new(spriteOffset:4000, defaultPalette:3);
        entry. = {
            widthChars: (SES.resolutionWidth / 6)->floor,
            heightChars: 1,
            editable : true,
            x: 6,
            y: SES.resolutionHeight - 8               
        };



        callbackID = SES.Input.addCallback(
            device: SES.Input.DEVICES.KEYBOARD,
            callback:::(event, text, key) {
                when(event != SES.Input.EVENTS.KEY_DOWN) empty;
                if (key == SES.Input.KEYS.RETURN) ::<= {
                    ses_native__debug_context_query(a:entry.text => String);
                    entry.text = '';
                    entry.scrollY = 0;
                    entry.moveCursor(x:0, y:0); 
                };
            }
        );                
    };
        
    ses_native__debug_context_enter(a:onDebugPrint, b:onDebugClear, c:onDebugInit);
    

    // normal                
    SES.Palette.set(
        index: 0,
        colors: [
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0.6, 0.6, 0.6]
        ]
    );

    // code
    SES.Palette.set(
        index: 1,
        colors: [
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [0.87, 0.8, 0.5]
        ]
    );

    // error
    SES.Palette.set(
        index: 2,
        colors: [
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [1, 0.3, 0.3]
        ]
    );

    // enter
    SES.Palette.set(
        index: 3,
        colors: [
            [0, 0, 0],
            [0, 0, 0],
            [0, 0, 0],
            [1, 1, 1]
        ]
    );     

    // for debug context


    [::] {
        forever(do:::{
            when(ses_native__debug_context_is_done()) send();
            ses_native__debug_context_update();                                   
        });
    };
    SES.Input.removeCallback(id:callbackID, device:SES.Input.DEVICES.KEYBOARD);
    entry.editable = false;
    display.text = '';
    entry.text = '';
    cursor.text = '';
    ses_native__debug_context_leave();
    inDebugContext = false;         


};


return class(
    name : 'SES.Debug',
    define::(this) {
        this.interface = {
            'breakpoint' : debug
        };
        
    }
).new();


