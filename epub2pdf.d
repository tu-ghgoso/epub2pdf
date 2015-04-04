import std.stdio;
import std.path;
import std.file;
import std.process;
import std.xml;
import std.string;

static int main(string[] args) {
	if(args.length!=3) {
		stderr.writeln("Usage: ", args[0], " <EPUB> <PDF>");
		return -1;
	}
	auto epubFile=absolutePath(args[1]);
	auto pdfFile=absolutePath(args[2]);
	auto tmpDir=pdfFile~".tmp";
	writeln("Creating temp directory ", tmpDir);
	mkdirRecurse(tmpDir);
	scope(exit) {
		writeln("Removing temp directory ", tmpDir);
		rmdirRecurse(tmpDir);
	}
	chdir(tmpDir);
	if(wait(spawnProcess(["/usr/bin/unzip", "-q", epubFile]))!=0) {
		stderr.writeln("Cannot extract ", epubFile);
		return -1;
	}
	auto mime=readText("mimetype");
	if(mime!="application/epub+zip") {
		stderr.writeln("Wrong mimetype: ", mime);
		return -1;
	}

	string[] rootfiles;
	auto conts=readText("META-INF/container.xml");
	auto contspars=new DocumentParser(conts);
	contspars.onStartTag["rootfile"]=(ElementParser x) {
		rootfiles~=x.tag.attr["full-path"].idup;
	};
	contspars.parse();

	if(rootfiles.length==0) {
		stderr.writeln("No rootfiles found");
		return -1;
	}
	if(rootfiles.length>1) {
		stderr.writeln("More than 1 rootfiles not supported.");
		return -1;
	}

	auto opfs=readText(rootfiles[0]);
	auto opfspars=new DocumentParser(opfs);

	string[string] id2href;
	string[] items;

	string publisher, description, creator, title;
	opfspars.onEndTag["dc:publisher"]=(in Element e) {
		publisher=e.text();
	};
	opfspars.onEndTag["dc:description"]=(in Element e) {
		description=e.text();
	};
	opfspars.onEndTag["dc:creator"]=(in Element e) {
		creator=e.text();
	};
	opfspars.onEndTag["dc:title"]=(in Element e) {
		title=e.text();
	};
	opfspars.onStartTag["manifest"]=(ElementParser x) {
		x.onStartTag["item"]=(ElementParser ep) {
			id2href[ep.tag.attr["id"]]=ep.tag.attr["href"];
		};
		x.parse();
	};
	opfspars.onStartTag["spine"]=(ElementParser x) {
		x.onStartTag["itemref"]=(ElementParser ep) {
			items~=ep.tag.attr["idref"];
		};
		x.parse();
	};
	opfspars.parse();

	chdir(dirName(rootfiles[0]));

	string[] parts;
	foreach(item; items) {
		auto href=id2href[item];
		auto part=href~"-tmp.pdf";
		if(wait(spawnProcess(["html2pdf", href, part]))!=0) {
			stderr.writeln("Error converting: ", href);
		} else {
			writeln("Finish converting: ", href);
			parts~=part;
		}
	}

	string[] cmd=["pdfunite"];
	cmd~=parts;
	cmd~=pdfFile;
	if(wait(spawnProcess(cmd))!=0) {
		stderr.writeln("Error calling pdfunite.");
		return -1;
	}
	writeln("Finished.");
	return 0;
}
