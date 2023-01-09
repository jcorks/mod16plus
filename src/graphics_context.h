#ifndef H_SES_GRAPHICS_CONTEXT__INCLUDED 
#define H_SES_GRAPHICS_CONTEXT__INCLUDED 

typedef struct sesGraphicsContext_t sesGraphicsContext_t;
typedef struct sesGraphicsContext_Layer_t sesGraphicsContext_Layer_t;

#define SES_GRAPHICS_CONTEXT__LAYER_COUNT 128


sesGraphicsContext_t * ses_graphics_context_create();

void ses_graphics_context_destroy(sesGraphicsContext_t *);




sesGraphicsContext_Layer_t * ses_graphics_context_get_layer(sesGraphicsContext_t *, uint8_t layer);

void ses_graphics_context_render(sesGraphicsContext_t *);










sesGraphicsContext_Layer_t * ses_graphics_context_layer_add_sprite(sesGraphicsContext_Layer_t *, sesCartridge_Sprite_t *);

sesGraphicsContext_Layer_t * ses_graphics_context_layer_add_background(sesGraphicsContext_Layer_t *, sesCartridge_Background_t *);





#endif
