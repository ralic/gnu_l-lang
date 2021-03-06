typedef int Int__type;
typedef int Bool__type;
typedef void *Type__type;
typedef void* Label__type;
typedef void Void__type;
typedef struct {} * Symbol__type;

typedef struct {unsigned int len; char *content;} * String__type;

Symbol__type intern( const char * string);
Void__type print_Int_Dec( int i);
Void__type print_String( String__type s);
void maybe_flush( void);
String__type make_heap_string( char *s);

typedef struct struct0x0805eba0 *List__O__Form__C____type__type;
typedef void *Output_Descriptor__type;
typedef struct struct0x0805d238 *Form__type;
struct struct0x0805d238 {
Int__type form_type_;
};
typedef struct struct0x0805e968 *Compound_Form__type;
struct struct0x0805e968 {
Int__type form_type_;
Symbol__type head;
Int__type garbage_;
List__O__Form__C____type__type form_list;
};
struct struct0x0805eba0 {
Form__type head;
List__O__Form__C____type__type tail;
};
struct struct0x08092b88 {
Form__type car;
List__O__Form__C____type__type cdr;
};
void *(alloc_cons_cell)(void);
void (print_String)(String__type );
void (maybe_flush)(void);
Form__type (Id_Form)(Symbol__type );
Output_Descriptor__type (make_string_output_descriptor)(void);
String__type (string_output_descriptor_to_string)(Output_Descriptor__type );
Form__type (Compound_Form)(Symbol__type , List__O__Form__C____type__type );
void (toto)(void);
Output_Descriptor__type current_output_descriptor;
Output_Descriptor__type stdout_output_descriptor;
Output_Descriptor__type stderr_output_descriptor;
static struct { unsigned int length; char *content; } __string_1 = { 6, "Test3\n"};
static struct { unsigned int length; char *content; } __string_2 = { 6, "Test1\n"};
static struct { unsigned int length; char *content; } __string_3 = { 6, "Test2\n"};
static struct { unsigned int length; char *content; } __string_4 = { 4, "s: |"};
static struct { unsigned int length; char *content; } __string_5 = { 2, "|\n"};
