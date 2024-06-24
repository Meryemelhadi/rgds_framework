<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main" exclude-result-prefixes="w">
	<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes" omit-xml-declaration="yes"/>
	<xsl:template match="/">
		<p>
			<xsl:value-of select="w:document/w:body/w:p[normalize-space(.) != ''][1]"/>
		</p>
	</xsl:template>
</xsl:stylesheet>
