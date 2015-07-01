# Tutorial to use the TreeReader class
----------------------------------------------------------------------------

Remember the ugly times in which trees were not just accessible by variables names?

They are finished thanks to TreeReader.

A full doxygen can be found at http://test-lhcbbham.web.cern.ch/test-lhcbbham/dc/dde/classTreeReader.html

## Constructors

```
TreeReader ( TTree * tree )      // using an existing TTree
TreeReader ( TChain * chain )     // using an existing TChain
TreeReader ( string nametree )   // a name of a tree
```
In the latter case you'll have to add the filenames using AddFile(filename).

After constructing you can Initialize() the tree (Necessary only in the latter case).
When initialising information of the number, names, types of the variables will be gathered.


You can use the TreeReader to do exacly what its name says: read the tree.
This is done by using GetValue().

```
reader->GetEntry(i);
cout << reader->GetValue("some_variable") << endl;
```

N.B.: A variable must be selected using reader->GetEntry().

## Entrylist

You want to skip entries? Not a problem get a TEntrylist of your entries and you
can set it in the TreeReader by reader->SetEntryList(entryList).

## Copying and Cloning

One other possible thing you'll what to do it to copy the tree or create an empty tree with
all branches setup as your previous tree.


## Cloning :

```
TTree * newEmptyTree = reader->CloneTree ( newname );
```

notice that all branches are linked to the other tree.
So for example you can now do 

```
reader->GetEntry(i)
newEmptyTree->Fill()
```

and the current entry will be filled (no need to set any address!!). 
If you create new banches on the new tree you can use this "copy the old tree adding branches".

Copying:
This is the quickest way to copy the entire tree or just a subsample of it.

```
reader->CopyTree (TCut cuts="", double frac=-1., string name="") 
```

"cuts" are cuts to be applyed. Only entries which pass will be saved in the new tree.
"frac" is to select only a fraction of the tree only. e.g. I use frac = 1000 for testing (so it doesn't take long)
N.B.: it's always the first 1000 entries so it cannot be directly used to extract random subsamples.
"name" is a new name to be given to the new tree. By default has he same name as the mother tree.

And much more!!!
