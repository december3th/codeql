/**
 * @name Resolving XML external entity from a local source (experimental sinks)
 * @description Parsing user-controlled XML documents and allowing expansion of external entity
 *              references may lead to disclosure of confidential data or denial of service.
 *              (note this version differs from query `java/xxe` by including support for additional possibly-vulnerable XML parsers,
 *              and by considering local information sources dangerous (e.g. environment variables) in addition to the remote sources
 *              considered by the normal `java/xxe` query)
 * @kind path-problem
 * @problem.severity recommendation
 * @precision medium
 * @id java/xxe-local-experimental-sinks
 * @tags security
 *       experimental
 *       external/cwe/cwe-611
 */

import java
import XXELib
import semmle.code.java.dataflow.TaintTracking
import semmle.code.java.dataflow.FlowSources
import XxeLocalFlow::PathGraph

module XxeLocalConfig implements DataFlow::ConfigSig {
  predicate isSource(DataFlow::Node src) { src instanceof LocalUserInput }

  predicate isSink(DataFlow::Node sink) { sink instanceof UnsafeXxeSink }
}

module XxeLocalFlow = TaintTracking::Global<XxeLocalConfig>;

from XxeLocalFlow::PathNode source, XxeLocalFlow::PathNode sink
where XxeLocalFlow::flowPath(source, sink)
select sink.getNode(), source, sink, "Unsafe parsing of XML file from $@.", source.getNode(),
  "user input"
