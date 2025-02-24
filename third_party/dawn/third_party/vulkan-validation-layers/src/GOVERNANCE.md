## Vulkan Validation Layers Repository Management

# **Open Source Project – Objectives**

* Alignment with the Vulkan Specification
  - The goal is for validation layer behavior to enforce the vulkan specification on applications. Questions on specification
interpretations may require consulting with the Khronos Vulkan Workgroup for resolution
    - [Core Specification](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html)
    - [Header Files](https://www.khronos.org/registry/vulkan/#headers)
* ISV Enablement
  - Updates of validation layer functionality should be available in a timely fashion
  - Every effort will be made to be responsive to ISV issues with validation layers
* Cross Platform Compatibility
  - Continuous Integration: HW test farms operated by LunarG monitor various hardware/software platforms for Android and desktop
* Repo Quality
  - Repo remains in healthy state with all tests passing and good-quality, consistent codebase
  - Continuous Integration: Along with Github, HW test farms operated by LunarG perform pre-commit cloud testing
on pull-requests

# **Roles and Definitions**
* Contributor, Commenter, User
  - Submitting contributions, creating issues, or using the contents of the repository
* Approver
  - Experienced project members who have made significant technical contributions
  - Write control: Approve pull/merge requests (verify submissions vs. acceptance criteria)
* Technical Project Leads
  - Lead the project in terms of versioning, quality assurance, and overarching objectives
  - Monitor github issues and drive timely resolution
  - Designate new approvers
  - Ensure project information such as the Readme, Contributing docs, wiki, etc., kept up-to-date
  - Act as a facilitator in resolving technical conflicts
  - Is a point-of-contact for project-related questions

The technical project leads for this repository are multiple engineers at LunarG.

# **Acceptance Criteria and Process**
  - All source code to include Khronos copyright and license (Apache 2.0).
    - Additional copyrights of contributors appended
  - Contributions are via pull requests
    - Project leads will assigning approvers to contributor pull requests
    - Approvers can self-assign their reviewers
    - For complex or invasive contributions, Project Leads may request approval from specific reviewers
    - At least one review approval is required to complete a pull request
    - The goal is to be responsive to contributors while ensuring acceptance criteria is met and to facilitate their submissions
    - Approval is dependent upon adherence to the guidelines in [CONTRIBUTING.md](CONTRIBUTING.md), and alignment with
repository goals of maintainability, completeness, and quality
    - Conflicts or questions will ultimately be resolved by the project leads. You can submit questions or conflicts as a github issue or send email to info@lunarg.com
