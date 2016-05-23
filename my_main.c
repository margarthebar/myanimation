/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser, 
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  frames: set num_frames (in misc_headers.h) for animation

  basename: set name (in misc_headers.h) for animation

  vary: manipluate knob values between two given frames
        over a specified interval

  set: set a knob to a given value
  
  setknobs: set all knobs to a given value

  push: push a new origin matrix onto the origin stack
  
  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix 
                     based on the provided values, then 
		     multiply the current top of the
		     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a 
		    temporary matrix, multiply it by the
		    current top of the origins stack, then
		    call draw_polygons.

  line: create a line based on the provided values. Store 
        that in a temporary matrix, multiply it by the
	current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the image live
  
  jdyrlandweaver
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "misc_headers.h"
#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"

//struct vary_node *knobs[num_frames];

/*======== void first_pass()) ==========
  Inputs:   
  Returns: 

  Checks the op array for any animation commands
  (frames, basename, vary)
  
  Should set num_frames and basename if the frames 
  or basename commands are present

  If vary is found, but frames is not, the entire
  program should exit.

  If frames is found, but basename is not, set name
  to some default value, and print out a message
  with the name being used.

  jdyrlandweaver
  ====================*/
void first_pass() {
  int i, f, b, v;
  f = -1;
  b = -1;
  v = -1;
  for(i=0; i<lastop; i++){
    if(op[i].opcode==FRAMES){
      f = 0;
      num_frames = op[i].op.frames.num_frames;
      printf("frames!\n");
    }else if(op[i].opcode==BASENAME){
      b = 0;
      strcpy(name,op[i].op.basename.p->name);
    }else if(op[i].opcode==VARY){
      v = 0;
    }
  }
  if(v==0 && f!=0){
    //entire program exit
    printf("Error: vary command found with no frame command\n");
  }
  if(f==0 && b!=0){
    strcpy(name,"framefile");
    printf("default basename selected: 'framefile'");
  }
  if(f!=0){
    num_frames = 1;
  }
  printf("num_frames: %d\n", num_frames);
}

/*======== struct vary_node ** second_pass()) ==========
  Inputs:   
  Returns: An array of vary_node linked lists

  In order to set the knobs for animation, we need to keep
  a seaprate value for each knob for each frame. We can do
  this by using an array of linked lists. Each array index
  will correspond to a frame (eg. knobs[0] would be the first
  frame, knobs[2] would be the 3rd frame and so on).

  Each index should contain a linked list of vary_nodes, each
  node contains a knob name, a value, and a pointer to the
  next node.

  Go through the opcode array, and when you find vary, go 
  from knobs[0] to knobs[frames-1] and add (or modify) the
  vary_node corresponding to the given knob with the
  appropirate value. 

  05/17/12 09:29:31
  jdyrlandweaver
  ====================*/
struct vary_node ** second_pass(struct vary_node *knobs[num_frames]) {
  int i, n, start_frame, end_frame, done, empty;
  double start_val, end_val, increment, val;
  empty = 0;
  val = 0;
  int elements[num_frames];
  for(i=0; i<num_frames; i++){
    elements[i] = 0;
  }
  for(i=0; i<lastop; i++){
    start_frame = op[i].op.vary.start_frame;
    end_frame = op[i].op.vary.end_frame;
    start_val = op[i].op.vary.start_val;
    end_val = op[i].op.vary.end_val;
    increment = (end_val-start_val)/(end_frame-start_frame);
    done = -1;
    if(op[i].opcode==VARY){
      struct vary_node *other_nodes[num_frames];
      for(n=0; n<num_frames; n++){
	if(n>=start_frame && n<=end_frame){
	  if(elements[n]==0){
	    elements[n] = 1;
	    struct vary_node first_node;
	    strncpy(first_node.name,op[i].op.vary.p->name,strlen(op[i].op.vary.p->name));
	    first_node.value = val;
	    first_node.next = NULL;
	    val+=increment;
	    knobs[n] = &first_node;
	    if(n<4){
	      printf("n: %d\n",n);
	      printf("name: %s\n",knobs[n]->name);
	      printf("val: %lf\n",val);
	      printf("increment: %lf\n",increment);
	      printf("first_node->value: %lf\n",first_node.value);
	      printf("modified knob: %lf\n\n",knobs[n]->value);
	    }
	  }else{
	    if(n==0){
	      printf("knobs[0]->name: %s\n\n",knobs[0]->name);
	    }
	    int num = 2;
	    //going through a linked list...
	    //keep track of first node. This should not change!
	    //keep track of current node
	    //add new node to tail
	    other_nodes[n] = malloc (sizeof(struct vary_node));
	    //struct vary_node other_node;
	    struct vary_node *next_node = malloc(sizeof(struct vary_node));
	    if(next_node == other_nodes[n]){
	      printf("conspiracy!!!\n");
	    }
	    if(n<4){
	      //if(/*&*/other_node==knobs[n]){///WHHHHYYYYYYYYYYY????????????
	      if(other_nodes[n]==knobs[n]){
		printf("same node\n");
	      }
	      printf("knobs[n]->name: %s\n\n",knobs[n]->name);
	    }
	    sprintf(other_nodes[n]->name,"%s",op[i].op.vary.p->name);
	    if(n<4){
	      if(other_nodes[n]==knobs[n]){
		printf("same node\n");
	      }
	      printf("knobs[n]->name: %s\n\n",knobs[n]->name);
	    }
	    other_nodes[n]->value = val;
	    other_nodes[n]->next = NULL;
	    val+=increment;
	    if(n<4){
	      printf("knobs[%d]->name: %s\n",n,knobs[n]->name);
	      printf("  elements: %d\n",elements[n]);
	      printf("  other_nodes[n]->name: %s\n",other_nodes[n]->name);
	    }
	    if(elements[n]==0){
	      knobs[n] = other_nodes[n];
	    }else if(elements[n]==1){
	      knobs[n]->next = other_nodes[n];
	      if(n<4){
		printf("  this tail node name: %s\n",knobs[n]->next->name);
	      }
	    }else{
	      printf("in here\n");
	      struct vary_node *this_node = knobs[n]->next;
	      while(num < elements[n]){
		this_node = this_node->next;
	      }
	      if(num==elements[n]){
		this_node->next = other_nodes[n];
	      }
	       if(n<4){
		printf("  tail node name: %s\n",this_node->next->name);
	      }
	    }
	    if(n<4){
	     printf("  final knobs[%d] name: %s\n\n",n,knobs[n]->name);
	    }
	    elements[n]++;
	  }
	}
      }
    }
  }
  //printf("reached end of function\n");
  printf("[%s]\n",knobs[0]->next->name);
  printf("[%s]\n",knobs[1]->next->name);
  printf("[%s]\n",knobs[2]->next->name);
  printf("[%s]\n",knobs[3]->next->name);
  return knobs;
}


/*======== void print_knobs() ==========
Inputs:   
Returns: 

Goes through symtab and display all the knobs and their
currnt values

jdyrlandweaver
====================*/
void print_knobs() {
  
  int i;

  printf( "ID\tNAME\t\tTYPE\t\tVALUE\n" );
  for ( i=0; i < lastsym; i++ ) {

    if ( symtab[i].type == SYM_VALUE ) {
      printf( "%d\t%s\t\t", i, symtab[i].name );

      printf( "SYM_VALUE\t");
      printf( "%6.2f\n", symtab[i].s.value);
    }
  }
}


/*======== void my_main() ==========
  Inputs:
  Returns: 

  This is the main engine of the interpreter, it should
  handle most of the commadns in mdl.

  If frames is not present in the source (and therefore 
  num_frames is 1, then process_knobs should be called.

  If frames is present, the enitre op array must be
  applied frames time. At the end of each frame iteration
  save the current screen to a file named the
  provided basename plus a numeric string such that the
  files will be listed in order, then clear the screen and
  reset any other data structures that need it.

  Important note: you cannot just name your files in 
  regular sequence, like pic0, pic1, pic2, pic3... if that
  is done, then pic1, pic10, pic11... will come before pic2
  and so on. In order to keep things clear, add leading 0s
  to the numeric portion of the name. If you use sprintf, 
  you can use "%0xd" for this purpose. It will add at most
  x 0s in front of a number, if needed, so if used correctly,
  and x = 4, you would get numbers like 0001, 0002, 0011,
  0487

  05/17/12 09:41:35
  jdyrlandweaver
  ====================*/
void my_main( /*int polygons*/ ) {

  int i, f, j;
  double step;
  double xval, yval, zval, knob_value;
  struct matrix *transform;
  struct matrix *tmp;
  struct stack *s;
  screen t;
  color g;

  struct vary_node **knobs;
  struct vary_node *vn;
  char frame_name[128];

  //num_frames = 1;
  step = 5;
 
  g.red = 0;
  g.green = 255;
  g.blue = 255;

  first_pass();

  if(num_frames==-1){
    process_knobs();
  }else{
    /*
       1) Set the symbol table values based on the knob table from Pass II

       2) Go through the operations as usual, applying knobs when present

       3) At the end of each frame loop, save the image using the basename + the frame # (don't forget padding zeros)
    */
    struct vary_node *local_knobs[num_frames];
    knobs = second_pass(local_knobs);
    int r;
    for(r=0;r<num_frames;r++){
      if(r<4){
	printf("r: %d, name: %s, next: %s\n",r,knobs[r]->name,knobs[r]->next->name);
      }
      if(knobs[r]->value!=0){
	//printf("found value!\n");
      }
    }
    vn = knobs[0];
    int done;
    done = -1;
    for(f=0; f<num_frames; f++){
      print_knobs();
      printf("frame number: %d\n",f);
      //go through knobs checking to see if they are in the symtable, if not add them
      vn = knobs[f];
      int l;
      l = 1;
      while(l<lastsym){
	if(lookup_symbol(vn->name)==NULL){
	  add_symbol(vn->name, SYM_VALUE, &vn->value);
	}else{
	  set_value(lookup_symbol(vn->name), vn->value);
	}
	l++;
	if(l<lastsym){
	  vn = vn->next;
	}
      }
      
      for (i=0;i<lastop;i++) {
	s = new_stack();
	transform = new_matrix(4,4);
	tmp = new_matrix(4,4);

	switch (op[i].opcode) {

	case SPHERE:
	  add_sphere( tmp,op[i].op.sphere.d[0], //cx
		      op[i].op.sphere.d[1],  //cy
		      op[i].op.sphere.d[2],  //cz
		      op[i].op.sphere.r,
		      step);
	  //apply the current top origin
	  matrix_mult( s->data[ s->top ], tmp );
	  draw_polygons( tmp, t, g );
	  tmp->lastcol = 0;
	  break;

	case TORUS:
	  add_torus( tmp, op[i].op.torus.d[0], //cx
		     op[i].op.torus.d[1],     //cy
		     op[i].op.torus.d[2],    //cz
		     op[i].op.torus.r0,
		     op[i].op.torus.r1,
		     step);
	  matrix_mult( s->data[ s->top ], tmp );
	  draw_polygons( tmp, t, g );
	  tmp->lastcol = 0;
	  break;

	case BOX:
	  add_box( tmp, op[i].op.box.d0[0],
		   op[i].op.box.d0[1],
		   op[i].op.box.d0[2],
		   op[i].op.box.d1[0],
		   op[i].op.box.d1[1],
		   op[i].op.box.d1[2]);
	  matrix_mult( s->data[ s->top ], tmp );
	  draw_polygons( tmp, t, g );
	  tmp->lastcol = 0;
	  break;

	case LINE:
	  add_edge( tmp, op[i].op.line.p0[0],
		    op[i].op.line.p0[1],
		    op[i].op.line.p0[1],
		    op[i].op.line.p1[0],
		    op[i].op.line.p1[1],
		    op[i].op.line.p1[1]);
	  draw_lines( tmp, t, g );
	  tmp->lastcol = 0;
	  break;

	case MOVE:
	  //get the factors
	  xval = op[i].op.move.d[0];
	  yval =  op[i].op.move.d[1];
	  zval = op[i].op.move.d[2];

	  if(op[i].op.move.p!=NULL){
	    xval *= op[i].op.move.p->s.value;
	    yval *= op[i].op.move.p->s.value;
	    zval *= op[i].op.move.p->s.value;
	  }
	  transform = make_translate( xval, yval, zval );
	  //multiply by the existing origin
	  matrix_mult( s->data[ s->top ], transform );
	  //put the new matrix on the top
	  copy_matrix( transform, s->data[ s->top ] );
	  //free_matrix( transform );
	  break;

	case SCALE:
	  xval = op[i].op.scale.d[0];
	  yval = op[i].op.scale.d[1];
	  zval = op[i].op.scale.d[2];

	  if(op[i].op.move.p!=NULL){
	    xval *= op[i].op.move.p->s.value;
	    yval *= op[i].op.move.p->s.value;
	    zval *= op[i].op.move.p->s.value;
	  }
      
	  transform = make_scale( xval, yval, zval );
	  matrix_mult( s->data[ s->top ], transform );
	  //put the new matrix on the top
	  copy_matrix( transform, s->data[ s->top ] );
	  //free_matrix( transform );
	  break;

	case ROTATE:
	  xval = op[i].op.rotate.degrees * ( M_PI / 180 );

	  if(op[i].op.move.p!=NULL){
	    xval *= op[i].op.move.p->s.value;
	  }

	  //get the axis
	  if ( op[i].op.rotate.axis == 0 ) 
	    transform = make_rotX( xval );
	  else if ( op[i].op.rotate.axis == 1 ) 
	    transform = make_rotY( xval );
	  else if ( op[i].op.rotate.axis == 2 ) 
	    transform = make_rotZ( xval );

	  matrix_mult( s->data[ s->top ], transform );
	  //put the new matrix on the top
	  copy_matrix( transform, s->data[ s->top ] );
	  //free_matrix( transform );
	  break;

	case PUSH:
	  push( s );
	  break;
	case POP:
	  pop( s );
	  break;
	case SAVE:
	  save_extension( t, op[i].op.save.p->name );
	  break;
	case DISPLAY:
	  display( t );
	  break;
	case FRAMES://done
	  num_frames = op[i].op.frames.num_frames;
	  break;
	case BASENAME://done
	  strcpy(name,op[i].op.basename.p->name);
	  break;
	case VARY:
	  //VARY C0DE
	
	  break;
	case SET:
	  //SET CODE
	  break;
	case SETKNOBS:
	  //SETKNOBS CODE
	  break;
	}
      }
  
      free_stack( s );
      free_matrix( tmp );
      free_matrix( transform );
      //save file
      sprintf(frame_name, "%s%04d.png",name,f);
      printf("%s\n",frame_name);
      save_extension( t, frame_name);
      clear_screen(t);
      //knobs = second_pass();
      //printf("end knobs[1]: %s\n",knobs[1]->name);
    }
  }
}

void process_knobs(){
  printf("in process_knobs\n");
}
