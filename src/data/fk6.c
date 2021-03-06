#include "data/fk6.h"

FK6 * fk6_create() {
    // allocate the structure if necessary
    FK6 * fk6 = calloc( 1, sizeof(FK6) );
    assert( fk6 );

    // allocate the metadata array
    fk6->fields = calloc( 2, sizeof(FK6_Field));
    assert(fk6->fields);

    return fk6;
}

int fk6_load_fields( FK6* fk6, FILE* readme, const char* header ) {

    // find the metadata section
    if(
        scan_line(readme, header) < 0 ||
        scan_line(readme, SEPARATOR) < 0 ||
        scan_line(readme, SEPARATOR) < 0
    ) {
        perror( "Invalid format" );
        exit(1);
    }

    // read rows from the file
    int n = 0;
    FK6_Field *field = NULL;
    int separator_length = strlen( SEPARATOR );
    while (1) {
        size_t size = 0;
        char *line = NULL;
        char start[4] = "\0\0\0", end[4] = "\0\0\0";

        // check for end of file and end of record
        if (getline(&line, &size, readme) == -1)
            return 1;
        else if (strncmp(line, SEPARATOR, separator_length) == 0)
            break;

        // if the second column is blank the row is an addendum to the explanation
        if( !get_value(line, 5, 8, end) ) {
            // read and append the rest of the explanation
            char rest[100] = " ";
            get_value(line, 35, strlen(line), rest + 1);
            strncat( field->Explanations, rest, 100-strlen(field->Explanations)-1 );
        }

        // otherwise create the field and parse data into it
        else {
            // flush the field
            if( field != NULL ) {
                fk6_add_field(fk6, field);
//                fk6_print_field( field, stdout );
                n++;
                free( field ); // TODO I should just really load this in place rather than making these copies
            }

            // allocate a new field structure
            field = malloc( sizeof(FK6_Field) );
            memset( field, 0, sizeof(FK6_Field) );
            assert( field != NULL );

            // read field from file
            field->end = atoi(end);

            // starting byte is absent in the case of single byte fields
            get_value(line, 1, 4, start);
            if( strcmp("", start) == 0 )
                field->start = field->end;
            else field->start = atoi( start );

            get_value(line, 10, 14, field->Format);
            get_value(line, 16, 22, field->Units);
            get_value(line, 23, 34, field->Label);
            get_value(line, 35, strlen(line), field->Explanations);
        }

        free(line);
    }
    // flush the field
    if( field != NULL ) {
        fk6_add_field(fk6, field);
        free( field );
    }
    return 0;
} // TODO it seems the column indices are not very static, looking between vizier catalogs. I should switch to a whitespace splitting approach...

void fk6_add_field( FK6 * fk6, FK6_Field * field ) {
    size_t n = fk6->cols;

    // resize whenever n gets larger than a power of 2
    if( n>1 && !(n&(n-1)) )
        fk6->fields = realloc(
                fk6->fields, 2*n*sizeof(FK6_Field) );
    assert(fk6->fields);

    (fk6->fields)[fk6->cols] = *field;
    fk6->cols++;
    // TODO don't allocate field, instantiate it in place
}

FK6_Field * fk6_get_field(FK6 * fk6, const char * label ) {
    for(int n=0; n<fk6->cols; n++) {
        FK6_Field * field = &(fk6->fields[n]);
        if( strcmp(label, field->Label)==0 )
            return field;
    }
    return NULL;
}

FK6_Field * fk6_get_index(FK6 * fk6, const int index ) {
    return &(fk6->fields[index]);
}

int fk6_get_value( char * line, FK6_Field * field, void * dest ) {
    char * buffer = NULL;
    int count = 0;

    // check for a string type so we can avoid an unnecessary buffer allocation
    if( field->Format[0] == 'A' )
        return get_value( line, field->start-1, field->end, (char*)dest );

    // allocate intermediate buffer and copy value string into it
    buffer = (char*) calloc( (size_t)(field->end - field->start + 2), sizeof(char) );
    count = get_value( line, field->start-1, field->end, buffer );

    // convert and set catalog value
    if( field->Format[0] == 'I' )
        *((long int*)dest) = (long int) atoi( buffer );

    else if( field->Format[0]=='F' )
        *((double*)dest) = (double) atof( buffer );
    // note: we might want to use of the precision info in the Field format...

    free( buffer );
    return count;
}

void fk6_print_field( FK6_Field * f, FILE * file ) {
    fprintf(file, "%s (%s, %s, %d-%d) : %s\n",
            f->Label, f->Units, f->Format, f->start, f->end, f->Explanations);
    fflush(file);
}

void fk6_free( FK6 * fk6 ) {
    assert( fk6 );
    free(fk6->fields);
    fk6->fields = 0;
}
