@include(css/variables,skin and device)
@variable(boxLength="300px")
@variable(fileUploadboxLength="100px")

/* box lengths */
input.text, input.password, textarea.text{
	width:@variable(boxLength);
}

input.text, input.other_text {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
	
	width:@variable(boxLength);
}

input.password {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
}

input.radio {
}

select.select {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
}

textarea.text {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
}

/* date 
  ----- */
div.input_date {
}

input#input_year {
	width:4em;
}

/* international phone
  --------------------*/
span.input_phone {
	width:@variable(boxLength);
}

input#phone_area {
	width:2.5em;
}

input#phone_local_number {
	width:150px;
}

input#phone_int_code {
	width:3em;
}

/* file upload 
   ----------- */
input.file {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
}

/* image upload 
   ------------ */
div.image_update_box, span.image_update_box {
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
	width:380px;
	padding:5px;
	overflow:auto;
}

input.file_image {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
	width:400px;
}

input.file_image_replace {
	background-color:#eeeeee;
	color:#333333;
	border: solid 1px #999999;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
	width:300px;
	margin-left:5px;
}

input.radio_image {
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
}
span.radio_image {
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
}

/* thumbnail (image replace) */
img.file_image {
	float:right;
	position:relative;
	left:-0em;
	z-index:2;
}

/* Rich text 
   --------- */
table.richText {
	width:420px;
}

table.toolbar {
	border:solid 1px black;
	background-color:threedface;
}

table.toolbar tr td
{
	border:solid 1px threedface;
	background-color:threedface;
}

table.toolbar tr td.separator
{
	border-left:solid 1px inset;
}

div.rich_text {
	background-color:#eeeeee;
	width:400px;
	overlap:scroll;
	word-wrap:break-word;
}

/* rich text content */
div.rich_text, div.rich_text p, div.rich_text ol, div.rich_text ul, div.rich_text font {
	font-family : Arial, Helvetica, sans-serif;
	font-size : 12px;
	font-weight: normal;
	color: #444444;
}


/* choice list
  ------------ */
span.choice_list_input {
}

ul.choice_level_0 {
	margin-left:0px;
}

ul.choice_level_1 {
}

ul.choice_level_2 {
}

li.choice_level_0 {
}

li.choice_level_1 {
}

li.choice_level_2 {
}

input.checkbox {
}

.checkbox_label {
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
}

.checkbox_label_header {
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: bold;
}

/* view dependent styles */
span.required
{
	color: red;
}

.form_label {
	color:#3E5670;
	font-family : Arial, Helvetica, sans-serif;
	font-size : x-small;
	font-weight: normal;
}

.form_field {
}

.form_field_error {
	border:solid 1px red;
}

/* help icon*/
img.help_icon {
	float:top;
}

.on .on_off, .off .off_on, .on .off .off_on  {
	display:inline; !important
}
.off .on_off,.on .off_on {
	display:none;
}